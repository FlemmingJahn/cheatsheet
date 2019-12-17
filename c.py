import optparse
import sys
import os
import yaml
import glob
import shutil

class Cheatsheet:
    sections = []  # Array of section containning the chaeats
    cheatSheetsPath = os.path.dirname(os.path.realpath(__file__)) + "/my_cheatsheets"  # Get the path to the cheatsheets
    currentCheatSheetFullPath = None
    copyArray = []

    def readYamlFile(self, fileName):
        if os.path.exists(fileName) and os.path.getsize(fileName) > 0:
            self.sections = yaml.load(fileName)

    def currentCheatSheetNameFullPathGet(self):
        return f"{self.cheatSheetsPath}/{self.currentCheatSheatName}.yaml"

    # Function for find a file that matches the file given by the used.
    # If multiple files matches, then print the list of files, and exit
    def findFileName(self):
        self.currentCheatSheatName = self.cheatSheetNameGet()
        self.storeLastCheatName()

        if self.currentCheatSheatName != None:
            if os.path.exists(self.currentCheatSheetNameFullPathGet()):
                return f"{self.currentCheatSheatName}.yaml"


        pwd = os.getcwd()
        os.chdir(self.cheatSheetsPath)
        if self.currentCheatSheatName == None:
            files = glob.glob('*.yaml') # Get all cheatsheets
        else:
            files = glob.glob(f"{self.currentCheatSheatName}*.yaml")
        os.chdir(pwd)

        if not files and self.currentCheatSheatName != None:
            # Create a new cheatSheet
            print("New cheatSheet named #{cheatName} created")
            return f"#{currentCheatSheatName}.yaml"

        if not files:
            print ("No cheatsheet files found")
            sys.exit()

        if self.currentCheatSheatName == None and len(files) == 1:
            f = files[0].gsub("\.yaml", "")
            print(f"One cheatsheet file found : {f}")
            sys.exit()

        if len(files) > 1:
            row, col = shutil.get_terminal_size(fallback=(80, 24))
            totalLen = 0
            print("Multiple cheatsheet files found. Please select one of the below:")
            for file in files:
                # Print as many cheatsheets as possible horizontal as possible for the terminal
                f = file.split(".")[0]
                totalLen += 20
                if totalLen >= row:
                    print("\n")
                totalLen = 20

                print("{:20s}".format(f), end = '')

            print("\n")
            sys.exit()

        # Only one file found.
        return f"{files[0]}"

    currentCheatSheatName = ""
    scriptPath = os.path.dirname(sys.argv[0])
    lastCheatSheet = f"{scriptPath}/cheatsheet.last"


    # If the first argument is a number then we copy the corresponding copyText from the last sheet that was read
    def cheatSheetNameGet(self):
        if unknownargs and unknownargs[0].isdecimal() == False:
            # user provided cheatsheet and not a copy request (a number)
            return unknownargs[0]

        # If the user didn't provide a cheat sheet name, will we list all possible cheatsheet
        if len(sys.argv) == 1:
            return None

        if not os.path.exists(self.lastCheatSheet):
            print("No cheatsheeet has been read yet - Can't do copytext until a sheet has been read.\n")
            sys.exit()

        with open(self.lastCheatSheet) as infile:
            lastCheatSheet = yaml.safe_load(infile)
            return lastCheatSheet


    def storeLastCheatName(self):
        with open(self.lastCheatSheet, "w") as file:
            yaml.dump(self.currentCheatSheatName, file)

    def readCheatSheet(self):
        with open(self.currentCheatSheetNameFullPathGet()) as infile:
            return yaml.safe_load(infile)

    def sectionLengthMax(self):
        maxLen = 0
        cheats = self.readCheatSheet()
        for cheat in cheats:
            for section in cheat.keys():
                if section.__len__() > maxLen:
                    maxLen = section.__len__()
        return maxLen

    def lengthsMax(self):
        sectionMaxLen = 0
        descrMaxLength = 0
        copyTxtMaxLength = 0
        for sections in self.readCheatSheet():
            for section in sections.keys():
                for cheat in sections[section]:
                    if section.__len__() > sectionMaxLen:
                        sectionMaxLen = section.__len__()

                    if cheat['Descr'] != None and cheat['Descr'].__len__() > descrMaxLength:
                        descrMaxLength = cheat['Descr'].__len__()

                    if cheat['CopyTxt'] != None and cheat['CopyTxt'].__len__() > copyTxtMaxLength:
                        copyTxtMaxLength = cheat['CopyTxt'].__len__()

        return sectionMaxLen, descrMaxLength, copyTxtMaxLength

    def printHeader(self, section, descrMaxLength, copyTxtMaxLength):
        totalLength = descrMaxLength + copyTxtMaxLength + 15 # Add 15 for spaces and section number and etc.
        sectionLength = section.__len__()

        if totalLength % 2:
            totalLength += 1

        if sectionLength % 2:
            section += " "
            sectionLength += 1

        dashLength = round((totalLength - sectionLength) / 2)

        text = "=" * dashLength + f" {section} " + "=" * dashLength
        print(text)

    def copyTxtToClipboard(self):
        userInput = unknownargs[0]
        if userInput.isdecimal():  # User asked for a copy (Numbered)
            userInputIndex = int(userInput) - 1

            allCheats = self.readCheatSheet()

            if userInputIndex >= len(allCheats):
                print (f"Index:{userInput} out of range")
            else:
               copyTxt = list(allCheats[userInputIndex].values())[0][0]['CopyTxt']
               pyperclip.copy(copyTxt)
               print (f"{copyTxt} copied to clip-board")

            sys.exit()


        for sections in allCheats:
            for section in sections.keys():
                for cheat in sections[section]:
                    descr   = str(cheat['Descr'] or '')
                    copyTxt = str(cheat['CopyTxt'] or '')
                    self.copyArray.append(copyTxt)

    def printCheatsheet(self):
        allCheats = self.readCheatSheet()
        sectionNumber = 1
        sectionMaxLen, descrMaxLength, copyTxtMaxLength = self.lengthsMax()

        self.copyArray = []
        for sections in allCheats:
            for section in sections.keys():
                self.printHeader(section, descrMaxLength, copyTxtMaxLength )
                for cheat in sections[section]:
                    descr   = str(cheat['Descr'] or '')
                    copyTxt = str(cheat['CopyTxt'] or '')
                    self.copyArray.append(copyTxt)

                    print(f"{sectionNumber:<{2}} - {descr:{descrMaxLength}} : {copyTxt}")
                    sectionNumber += 1
            print()



helpTxt = "If you like me are working with a Linux terminal all day long,\n" \
          "but also have as poor a memory as me,\n"\
          "you probably have a document with all the commands and cheats you use. This script can help you out,\n"\
          "and can be used to easily store you cheats and easily retreat them again. This all from the command line.\n"\
          "Each cheatsheet is stored in it's own file (a yaml text file, which you can edit in any text editor). You can order your cheats in sections with the -s option\n"\
          "For each cheat you can have a 'copyText', which can we used to easy get the 'copytext' into the clipboard,\n"\
          "so you can paste it e.g. into your command line\n"\
          "\n"\
          "Examples:\n"\
          "\n"\
          "Example for printing help : c --help\n" \
          "Usage: c [cheatSheet] [cpIndex] [option] [<String: Description of what to remember>]\n" \
          "-s, --section  <string>          Section header.\n" \
          "-c, --copytext <string>          Insert new 'copy' text.\n" \
          "-d, --description                Prints a description of this script.\n" \
          " " \
          "Example : You want to store a cheat for how to checkout a project from git.\n" \
          'Command for adding cheat : c git -h "projects" "How to checkout a new project" -c "git clone [url]"\n' \
          "\n " \
          "Example : Show your 'cheats' for git\n" \
          "Command for showing cheat : c git\n" \
          "Result :  ===================== projects ========= ============     ================================= default ================================\n" \
          "Result :  0  How to checkout a new project :  git clone [url]       1  Cleanup everything (done at top dir) :  git clean -df; git reset --hard\n" \
          "\n " \
          "Example : Get the 'copy text' into clipboard\n" \
          "Command for getting copy text '1' from the last shown cheats : git 1\n" \
          "Result : 'git clean -df; git reset --hard' copied to clipboard, now you can paste the 'git clean -df; git reset --hard' command"



import pyperclip



import argparse
parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('-v', '--version', action='version', version='%(prog)s 1.0', help="Show program's version number and exit.")
parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS, help=helpTxt)
args, unknownargs = parser.parse_known_args()


c = Cheatsheet()

c.findFileName()
c.copyTxtToClipboard()
c.printCheatsheet()






