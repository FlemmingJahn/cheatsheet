import optparse
import sys
import os

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


import argparse
parser = argparse.ArgumentParser(add_help=False, formatter_class=argparse.RawTextHelpFormatter)

parser.add_argument('-v', '--version', action='version', version='%(prog)s 1.0', help="Show program's version number and exit.")
parser.add_argument('-h', '--help', action='help', default=argparse.SUPPRESS, help=helpTxt)
args, unknownargs = parser.parse_known_args()



cheatSheetName = unknownargs[0]

if cheatSheetName.isdecimal():
    scriptPath = os.path.dirname(sys.argv[0])

    lastCheatSheet = f"{scriptPath}/cheatsheet.last"
    if not os.path.exists(lastCheatSheet):
        print("No cheatsheeet has been read yet - Can't do copytext until a sheet has been read.\n")
        sys.exit()

    with open(lastCheatSheet) as json_file:
        cheatSheetName = json.load(json_file)

