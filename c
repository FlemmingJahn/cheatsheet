#!/usr/bin/env ruby
#
#    Copyright (C)
#
#    This program is free software: You can do whatever you like - no limitation.
#
#    Comments, suggestions, questions to:



if Gem::Specification::find_all_by_name('pry').any?
  require 'pry'      # For debugging
end

if Gem::Specification::find_all_by_name('clipboard').any?
  require 'clipboard'
end

require 'optparse' # For command line parsing
require 'io/console'
require 'yaml'

class Cheatsheet
  HDR_SPACING = 5           # Number of spaces between each "header section"
  attr_accessor :cpIndex    # Index for the line to copy to clipboard

  def initialize
    @maxLen   = {}          # Max line length within a section
    @linesCnt = {}
    @sections = []          # Array of section containning the chaeats
  end

  # Pinting header
  def printHeader hdrTxt, hdrLen
    hdr = " #{hdrTxt} "
    len = ((hdrLen - (hdr.length)) / 2) + 1 # +1 for rounding up
    len = 1 if len < 1 # Make sure that we don't "overrun"
    output = "\e[31m" # Coloring
    output += "=" * len
    output += hdr
    output += "=" * len
    output += " " * HDR_SPACING
    output += "\e[0m"
    print output
  end

  # Returning the copy index number for a given "copy text" string
  def getCopyIndex cpTxt
    cnt = 0
    @sections.each { |sec|
      key=sec.keys[0]
      sec[key].each { |s|
    	if !s["CopyTxt"].nil?
          if cpTxt == s["CopyTxt"]
            return cnt
          end
          cnt += 1
        end
      }
    }
    return -1
  end

  def getCopyTxt cpIndex
    cnt = 0
    @sections.each { |sec|
      key=sec.keys[0]
      sec[key].each { |s|
    	if !s["CopyTxt"].nil?
          if cnt == cpIndex
            return s["CopyTxt"]
          end
          cnt += 1
        end
      }
    }
    return ""
  end

  # Print one line within the section
  def printLine copyText, descr, len
    output = ""

    # Calculate and print 2 digit copy index
    if copyText
      cpIndex = getCopyIndex(copyText)
      output   += sprintf("%-2d ", cpIndex)
    elsif descr
      output   += "-" + " " * 2
    end

    # Print the description
    output += "#{descr}" if descr

    if descr && copyText
      output += " : "
    end

    if copyText
      output += "\e[34m #{copyText}\e[0m"  # Coloring
      len    += 9  # +9 for the coloring characters above
    end

    # Append spaces so all lines becomes same length
    append = (2 + HDR_SPACING + len - output.length)
    output += " " * append if append > 0

    print output
  end

  # Find the max line length for all the lines within a section
  def findLinesMaxLength
    @sections.each { |h|
      maxLen = 0
      key = h.keys[0]

      h[key].each {|l|
        len = 6
        len += l["Descr"].length   if !l["Descr"].nil?
        len += l["CopyTxt"].length if !l["CopyTxt"].nil?
        maxLen = len if len > maxLen
      }
      @maxLen[key]=maxLen
      @linesCnt[key]=h[key].length
    }
  end


  def readYamlFile fileName
    if File.file?(fileName) && !File.zero?(fileName)
      @sections = YAML.load_file(fileName)
    end
  end

  def writeYamlFile f
    File.open(f, 'w') { |file|
     file.puts(@sections.to_yaml)
    }
  end

  # Print all section from "start" to "endpoint"
  def dumpIt start, endpoint
    maxLines = 0

    # Print the headers and figure out lines cnt for the section with most lines
    for i in start..endpoint
      return if @sections[i].nil?
      key = @sections[i].keys[0]
      printHeader(key, @maxLen[key])
      maxLines = @linesCnt[key] -1 if @linesCnt[key] > maxLines
    end
    print("\n")

    # Pritn all the lines in the sections
    lineNo = 0
    for lineNo in 0..maxLines
      for i in start..endpoint
        key = @sections[i].keys[0]
        l = @sections[i][key][lineNo]

        if !l.nil?
          printLine(l["CopyTxt"], l["Descr"], @maxLen[key])
	else
          printLine(nil, nil, @maxLen[key])
        end
      end
      print("\n")
    end
   end

  # Print out the sections
  def dump
    findLinesMaxLength # Find line count and lines length for all the sections

    currentLen = 0
    endPoint = @sections.length-1
    start = 0
    endPoint = 0
    while 1 do
      col, row = IO.console.winsize
#      puts("row:#{row}")
      # Figure out how many section that can be printed with the screen width
      for i in start..@sections.length-1
        key = @sections[i].keys[0]
        len = @maxLen[key] + HDR_SPACING
#        puts("row:#{row}, len:#{len}")
        if (row - len) < 0
           break
        end
        endPoint = i
        row -= len
      end

      #print the section that can fit within the screen width
      dumpIt(start, endPoint)

      # Continue with the next sections
      start = endPoint + 1
      if start > @sections.length-1
        break
      end
     end
   end

  # Function for find a file that matches the file given by the used.
  # If multiple files matches, then print the list of files, and exit
  def findFileName cheatName
    return "#{cheatName}.yaml" if File.file?("#{PATH}/#{cheatName}.yaml") && !cheatName.nil? # Exact match

    pwd = Dir.pwd
    Dir.chdir(PATH)
    files = Dir.glob("#{cheatName}*.yaml")
    Dir.chdir(pwd)

    if files.length == 0 && !cheatName.nil?
      # Create a new cheatSheet
      puts "New cheatSheet named #{cheatName} created"
      return "#{cheatName}.yaml"
    end

    if cheatName.nil?
      files = Dir.entries("#{PATH}").select { |str| str.include?('.yaml') }
    end

    if files.length == 0
      puts "No cheatsheet files found"
      exit
    end

    if cheatName.nil? && files.length == 1
      f = files[0].gsub("\.yaml", "")
      puts "One cheatsheet file found : #{f}"
      exit
    end

    if files.length > 1
      col, row = IO.console.winsize
      totalLen = 0
      puts "Multiple cheatsheet files found. Please select one of the below:"
      files.each do |file|
        # Print as many cheatsheets as possible horizontal as possible for the terminal
        f = file.gsub("\.yaml", "")
        totalLen += 20
        if totalLen >= row
          printf("\n")
          totalLen = 20
        end
        printf("%-20s", f)
      end
      printf("\n")
      exit
    end

    # Only one file found.
    return "#{files[0]}"
  end

  # Insert a new "cheat"
  def insert hdrTxt, descr, cpTxt
    newlines = []
    hdrTxt = "default" if hdrTxt.nil?
    found = false
    line = {"Descr"=>descr, "CopyTxt"=>cpTxt}
    @sections.each { |h|
      if h.has_key? hdrTxt
        h[hdrTxt].push(line)
        h[hdrTxt] = h[hdrTxt].uniq
        found = true
        break
      end
    }

    if (!found)
      newlines.push(line)
      newlines = newlines.uniq
      header= {hdrTxt=>newlines}
      @sections.push(header)
    end
    @sections = @sections.uniq
  end
end #End class



c = Cheatsheet.new()


#
# Option parser
#
def checkOptions options
  OptionParser.new do |opts|
    opts.banner = "Usage: c [cheatSheet] [cpIndex] [option] [<String: Description of what to remember>]"
    opts.version = 0.2

    opts.on("-s", "--section <string>", "Section header.") do |h|
      options[:header] = "#{h}"
    end

    opts.on("-c", "--copytext <string>", "Insert new \"copy\" text.") do |h|
      options[:copytext] = "#{h}"
    end

    opts.on("-d", "--description", "Prints a description of this script.") do
      puts "If you like me are working with a Linux terminal all day long, but also have as poor a memory as me, "
      puts "you probably have a document with all the commands and cheats you use. This script can help you out,  "
      puts "and can be used to easily store you cheats and easily retreat them again. This all from the command line."
      puts "Each cheatsheet is stored in it's own file (a yaml text file, which you can edit in any text editor). You can order your cheats in sections with the -s option"
      puts "For each cheat you can have a 'copyText', which can we used to easy get the 'copytext' into the clipboard,"
      puts "so you can paste it e.g. into your command line"
      puts ""
      puts "Examples:"
      puts ""
      puts "Example for printing help : c --help"
      puts "Usage: c [cheatSheet] [cpIndex] [option] [<String: Description of what to remember>]"
      puts "-s, --section  <string>          Section header."
      puts "-c, --copytext <string>          Insert new 'copy' text."
      puts "-d, --description                Prints a description of this script."
      puts ""
      puts "Example : You want to store a cheat for how to checkout a project from git."
      puts 'Command for adding cheat : c git -h "projects" "How to checkout a new project" -c "git clone [url]"'
      puts ""
      puts "Example : Show your 'cheats' for git"
      puts "Command for showing cheat : c git"
      puts "Result :  ===================== projects ========= ============     ================================= default ================================"
      puts "Result :  0  How to checkout a new project :  git clone [url]       1  Cleanup everything (done at top dir) :  git clean -df; git reset --hard"
      puts ""
      puts "Example : Get the 'copy text' into clipboard"
      puts "Command for getting copy text '1' from the last shown cheats : git 1"
      puts "Result : 'git clean -df; git reset --hard' copied to clipboard, now you can paste the 'git clean -df; git reset --hard' command"
      exit
    end
  end.order!
  return ARGV.shift
end


options = {}
cheatSheetName = checkOptions options

PATH = File.dirname($0) + "/my_cheatsheets" # Get the path to the cheatsheets


# If the first argument is a number then we copy the corrsponding copyText from the last sheet that was read
nameIsInt = Integer(cheatSheetName) rescue nil
if nameIsInt
  if !File.exist?("#{PATH}/cheatsheet.last")
    puts "No cheatsheeet has been read yet - Can't do copytext until a sheet has been read."
    exit
  end
  cheatSheetName = IO.read("#{PATH}/cheatsheet.last") #
  arg1 = nameIsInt
else
  arg1 = checkOptions options
end

file = c.findFileName(cheatSheetName)
IO.write("#{PATH}/cheatsheet.last", cheatSheetName) # Store that this is the latest cheatsheet that has been read.
c.readYamlFile "#{PATH}/#{file}"


argIsInt = Integer(arg1) rescue nil
if argIsInt
  cpIndex = arg1.to_i
  plainTxt = checkOptions options
else
  plainTxt = arg1
end

# Check that we have reach the last argument
anyMoreArg = checkOptions options
if  anyMoreArg
  puts "Unexpected input : #{anyMoreArg}"
  exit
end

if plainTxt.nil? && cpIndex
  cpTxt = c.getCopyTxt cpIndex

  if  cpTxt == ""
    puts "Error: No copytext found for number:#{cpIndex}"
  else
    puts "'#{cpTxt}' copied to clipboard"
    Clipboard.copy cpTxt
  end
  exit
end

if cheatSheetName.nil?
  exit
end

if !options[:header].nil? || !plainTxt.nil? || !options[:copytext].nil?
  c.insert options[:header], plainTxt, options[:copytext]
end

# We do always print out the cheats
c.dump

c.writeYamlFile "#{PATH}/#{file}"
