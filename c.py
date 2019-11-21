import optparse
import sys
import os
import yaml
import glob
import shutil

class Cheatsheet:
    sections = []  # Array of section containning the chaeats

    def readYamlFile(self, fileName):
        if os.path.exists(fileName) and os.path.getsize(fileName) > 0:
            self.sections = yaml.load(fileName)

    def getCheatSheetsPath(self):
        return os.path.dirname(os.path.realpath(__file__)) + "/my_cheatsheets"  # Get the path to the cheatsheets

    # Function for find a file that matches the file given by the used.
    # If multiple files matches, then print the list of files, and exit
    def findFileName(self, cheatName):
        if cheatName != None:
            if path.exists(cheatName):
                return f"{cheatName}.yaml"


        pwd = os.getcwd()
        os.chdir(self.getCheatSheetsPath())
        if cheatName == None:
            files = glob.glob('*.yaml') # Get all cheatsheets
        else:
            files = glob.glob(f"{cheatName}*.yaml")
        os.chdir(pwd)

        if not files and cheatName != None:
            # Create a new cheatSheet
            print("New cheatSheet named #{cheatName} created")
            return f"#{cheatName}.yaml"

        if not files:
            print ("No cheatsheet files found")
            sys.exit()

        if cheatName == None and len(files) == 1:
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


def cheatSheetNameGet():
    if unknownargs:
        # user provided cheatsheet
        cheatSheetName = unknownargs[0]

        if cheatSheetName.isdecimal():
            scriptPath = os.path.dirname(sys.argv[0])

        lastCheatSheet = f"{scriptPath}/cheatsheet.last"
        if not os.path.exists(lastCheatSheet):
            print("No cheatsheeet has been read yet - Can't do copytext until a sheet has been read.\n")
            sys.exit()

        with open(lastCheatSheet) as infile:
            return yaml.load(infile)

    return None

import argparse
parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('-v', '--version', action='version', version='%(prog)s 1.0', help="Show program's version number and exit.")
parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS, help=helpTxt)
args, unknownargs = parser.parse_known_args()


cheatSheetName = cheatSheetNameGet()



c = Cheatsheet()

file = c.findFileName(cheatSheetName)

with open(f"#{c.getCheatSheetsPath()}/cheatsheet.last", 'w') as outfile:
    yaml.dump(cheatSheetName, outfile)



