#!/usr/bin/env ruby
#
#    Copyright (C)
#
#    This program is free software: You can do whatever you like - no limitation.
#
#    Comments, suggestions, questions to:
#require 'pry'      # For debugging
require 'optparse' # For command line parsing
require 'io/console'
require 'clipboard'


$copyLines = []

class Line
  attr_accessor :text
  attr_accessor :copy
  attr_accessor :lineLen

  def initialize
    @type = nil
    @text = nil # Normal comment/test
    @copy = nil # Text that shall be numbered for copy posiblity
    @cpIndex = 0
    @lineLen = 0
  end

  def addCopyText string
    @copy = string
    $copyLines.push(string.tr("\n", ""))
    @cpIndex = $copyLines.length
  end

  def dump
    output = ""
    if @copy
      output += sprintf("%-3d", @cpIndex)
    else
      output += "   "
    end
    output += " #{@text}" if @text
    output += " :" if @text && @text != "" && @copy
    output += "\e[36m #{@copy}\e[0m" if @copy
    @lineLen = output.length
    @lineLen = @lineLen - 8 if @copy # Subtact the length of "\e[36m\e[0m" since these are not printed
    return output
  end
end


class Block
  attr_accessor :header
  attr_accessor :lineLenMax # The length of the line with max. length within the block

  HDR_LEN_MIN = 10
  HDR_SPACING = 5
  def initialize name
    @header = name
    @lines = []
    @lineLenMax = HDR_LEN_MIN
    @hdrLen = HDR_LEN_MIN
  end

  def lineCnt
    return @lines.length
  end

  def addLine newLine
    l = Line.new
    l.text, copyText = newLine.split("C::")
    l.addCopyText copyText if copyText
    l.dump # Call dump in oprder to update lineLen
    @lines.push(l)
    @lineLenMax = l.lineLen if @lineLenMax < l.lineLen
  end

  def write file
    file.write("H::#{@header}\n")
    @lines.each do |l|
      file.write(l.text)
      file.write("C::#{l.copy}") if l.copy
      file.write("\n")
    end
  end

  def dump hdrNo, lineNo
    output = ""
    if lineNo == 0
      hdr = " H:#{hdrNo} #{@header} "
      len = ((@lineLenMax - (hdr.length)) / 2)
      len = 1 if len < 1 # Make sure that we don't "overrun"
      output = "\e[31m"
      output += "=" * len
      output += hdr
      output += "=" * len
      output += " " * HDR_SPACING
      output += "\e[0m"
      @hdrLen = output.length - 8 # -8 for the coloring tags
      return output
    end
    len = 0
    if lineNo <= @lines.length
      output =  @lines[lineNo-1].dump
      len = @lines[lineNo-1].lineLen
    end
    output += " " * (@hdrLen - len)

 end
end


class Cheatsheet
  def initialize
    @blocks = []
  end

  def insertLine block, line
   if /^H::/.match(line)
     block.header = line.gsub(/^H::/,"")
     block.lineLenMax = block.header.length + 8 # + 8 is for the 2 spaces + minimum 2 "=" 1 and 4 charater for the header number
    else
      block.addLine line
    end
    return block
  end

  def parse file

    firstHeader = true
    b = Block.new("Default")

    rdLines = IO.read("#{file}")
    rdLines.each_line do |line|
      line = line.tr("\n", "")
      if /^H::/.match(line)
        if firstHeader
          b.header = "Default"
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

  def insert line, hdrNo
    if /^H::/.match(line)
      b = Block.new(line)
      if hdrNo
        @blocks.insert(hdrNo + 1, b)
      else
        @blocks.push(b)
      end
    end
    hdrNo = -1 if hdrNo.nil?
    insertLine @blocks[hdrNo], line
  end

  def write f
    File.open(f, 'w') { |file|
      @blocks.each do |blk|
        blk.write file
      end
    }
  end

  def dump
    col, row = IO.console.winsize
    size = col
    hdrArrLen = 0

    lineCnt = @blocks[0].lineCnt
    totalLineLen = getNextLen -1 # Get the first header length
    blkStart = 0
    for blkNo in 0..@blocks.length-1

      lineCnt = @blocks[blkNo].lineCnt if @blocks[blkNo].lineCnt > lineCnt
      totalLineLen += getNextLen blkNo
      if totalLineLen >= row || blkNo == @blocks.length-1
        dumpLines blkStart, blkNo, lineCnt

        lineCnt = 0
        blkStart = blkNo + 1
        totalLineLen = getNextLen blkNo
      end
    end
  end

  private

  def getNextLen blkNo
    if blkNo != @blocks.length-1
      return @blocks[blkNo + 1].lineLenMax + Block::HDR_SPACING
    else
      return @blocks[blkNo].lineLenMax + Block::HDR_SPACING
    end
  end

  def dumpLines blkStart, blkEnd, lineCnt
    for i in 0..lineCnt
      output = ""
      for blkNo in blkStart..blkEnd
        h = @blocks[blkNo]
        output += h.dump blkNo, i
      end
      puts output
    end
    puts "" # New line between headers
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

file = (getFile cheatSheetName)
file = "#{PATH}/#{file}"
IO.write("#{PATH}/cheatsheet.last", cheatSheetName) # Store that this is the latest cheatsheet that has been read.
cheat.parse file

argIsInt = Integer(arg1) rescue nil
if argIsInt
  hdrNo = arg1.to_i
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

if options[:header]
  cheat.insert options[:header], hdrNo
  writeFile = true
end

if options[:copytext]
  plainTxt  = "" if plainTxt.nil?
  plainTxt += options[:copytext]
end

if plainTxt.nil? && hdrNo
  if  $copyLines.length < hdrNo
    puts "Error: No copytext found for number:#{hdrNo}"
  else
    Clipboard.copy $copyLines[hdrNo -1]
    puts "#{$copyLines[hdrNo -1]} copied to clipboard"
  end
  exit
end


if plainTxt
  cheat.insert plainTxt, hdrNo
  writeFile = true
end

if writeFile
  cheat.write "#{file}"
  exit
end

cheat.dump
