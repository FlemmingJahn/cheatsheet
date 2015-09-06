#!/usr/bin/env ruby
#
#    Copyright (C)
#
#    This program is free software: You can do whatever you like - no limitation.
#
#    Comments, suggestions, questions to:
require 'pry'      # For debugging
require 'optparse' # For command line parsing
require 'io/console'
require 'clipboard'


$copyLines = []

class Line
  attr_accessor :text
  attr_accessor :copy

  def initialize
    @type = nil
    @text = nil # Normal comment/test
    @copy = nil # Text that shall be numbered for copy posiblity
    @cpIndex = 0
  end

  def addCopyText string
    @copy = string
    $copyLines.push(string.tr("\n", ""))
    @cpIndex = $copyLines.length
  end

  def print
    if @copy
      printf("%-3d", @cpIndex)
    else
      printf "   "
    end

    printf " #{@text}" if @text
    printf " :\e[36m #{@copy}\e[0m" if @copy
  end
end

class Block
  attr_accessor :header

  def initialize name
    @header = name
    @lines = []
  end

  def addLine newLine
    l = Line.new
    l.text, copyText = newLine.split("C::")
    l.addCopyText copyText if copyText
    @lines.push(l)
  end

  def lenMax
    maxLen = 0
    @lines.each do |l|
      lineLen = 0
      lineLen += l.text.length if l.text
      lineLen += l.copy.length if l.copy

      maxLen = lineLen if lineLen > maxLen
    end
    return maxLen + 5
  end

  def write file
    file.write("H::#{@header}\n")
    @lines.each do |l|
      file.write(l.text)
      file.write("C::#{l.copy}") if l.copy
      file.write("\n")
    end
  end

  def print hdrNo
    puts ""
    puts "=" * lenMax
    len = ((lenMax - @header.length) / 2)
    len = 0 if len < 0 # Make sure that we don't "overrun"
    printf " " * len
    puts "#{hdrNo} #{@header}"
    puts "=" * lenMax

    i = 0
    @lines.each do |l|
      l.print
    end
  end
end


class Cheatsheet
  def initialize
    @blocks = []
  end

  def insertLine block, line
    if /^H::/.match(line)
      block.header = line.gsub(/^H::/,"")
    else
      block.addLine line
    end
    return block
  end

  def parse file

    firstHeader = true
    b = Block.new("-")

    rdLines = IO.read("#{file}")
    rdLines.each_line do |line|
      line = line.tr("\n", "")
      if /^H::/.match(line)
        if firstHeader
          b.header = "-"
        else
          @blocks.push(b)
        end
        b = Block.new(line)
        firstHeader = false
      end
      b = insertLine b, line
    end
    @blocks.push(b)
  end

  def append line
    if /^H::/.match(line)
      b = Block.new(line)
      @blocks.push(b)
    end

    insertLine @blocks[-1], line
  end

  def write f
    File.open(f, 'w') { |file|
      @blocks.each do |blk|
        blk.write file
      end
    }
  end

  def print
    col, row = IO.console.winsize
    size = col
    i = 0
    hdrArrLen = 0
    @blocks.each do |h|
    #  hArr.push(h)

      if (h.lenMax + hdrArrLen) > col
        break
      end
    end

    @blocks.each do |h|
      h.print i
      i += 1
    end
  end
end

# Function for find a file that matches the file given by the used.
# If multiple files matches, then print the list of files, and exit
def getFile fileName
  return "#{fileName}.txt" if File.file?("#{PATH}/#{fileName}.txt") # Excat match


  Dir.chdir(PATH)
  files = Dir.glob("#{fileName}*.txt")

  if files.length == 0
    `touch #{fileName}.txt` # Create a new cheatSheet
    puts "New cheatSheet named #{fileName} created"
    return "#{fileName}.txt"
  end

  col, row = IO.console.winsize
  totalLen = 0

  if files.length > 1
    puts "Multiple files found. Please select one of the below:"
    files.each do |file|
      # Print as many cheatsheets as possible horizontal as possible for the terminal
      f = file.gsub("\.txt", "")
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
  return files[0]
end

def checkOptions options
  OptionParser.new do |opts|
    opts.banner = "Usage: m [cheatSheet] [hdrNo] [option] [<String: What to remember>]"
    opts.version = 0.1

    opts.on("-H", "--HEADER <string>", "Insert new header.") do |h|
      options[:header] = "H::#{h}"
    end

    opts.on("-C", "--COPYTEXT <string>", "Insert new \"coopy\" text.") do |h|
      binding.pry
      options[:copytext] = "C::#{h}"
    end

  end.order!
  return ARGV.shift
end

options = {}



cheat = Cheatsheet.new
# Parse the cheatSheet or print list of cheatSheets

cheatSheetName = checkOptions options

PATH = File.dirname($0) + "/my_cheatsheets" # Get the path to the cheatsheets

file = (getFile cheatSheetName)
file = "#{PATH}/#{file}"
cheat.parse file

arg1 = checkOptions options

argIsInt = Integer(arg1) rescue nil
if argIsInt
  hdrNo = arg1.to_i
  remSting = checkOptions options
else
  remString = arg1
end

# Check that we have reach the last argument
anyMoreArg = checkOptions options
if  anyMoreArg
  puts "Unexpected input : #{anyMoreArg}"
  exit
end

if remString.nil? && hdrNo
  Clipboard.copy $copyLines[hdrNo -1]
  exit
end


if options[:header]
  cheat.append options[:header]
  writeFile = true
end

if remString && options[:copytext]
  remString = remString + options[:copytext]
end

if remString && hdrNo.nil? || options[:copytext]
  cheat.append remString
  writeFile = true
end

if writeFile
  cheat.write "#{file}"
  exit
end

cheat.print
