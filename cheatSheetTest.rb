#!/usr/bin/env ruby
#
# Testcase for testing cheatsheet script
#
require 'pry'      # For debugging

testCheat = "newCheatsheet"
$testFile  = File.dirname($0) + "/my_cheatsheets/" + testCheat + ".txt"  # Get the path to the cheatsheets

system("rm -f #{$testFile}")

def systemTest cmd, expectedResult
  puts cmd
  result = `#{cmd}`

  if result.match(expectedResult).nil?
    puts "Error: result:#{result}, expected result:#{expectedResult}"
    exit
  end
end

def checkIfCreated
  puts "Error: New cheatSheet not created - #{$testFile}" if !File.exist?($testFile)
end

systemTest("c #{testCheat}", "New cheatSheet named #{testCheat} created") # Testing creating new cheatsheet
checkIfCreated


# Create and insert cheat text
systemTest("c #{testCheat} \"Ordinary Text\"", "")  # Create new cheatSheet
puts "Error: New cheatSheet not created" if !File.exist?($testFile)
checkIfCreated


# Insert Header
systemTest("c #{testCheat} -H \"New Header\"", "")  #
systemTest("c #{testCheat}", "1 New Header")  # Check that we can read out the header


# Insert Copy
systemTest("c #{testCheat} -C \"New copy text\"", "")  #
systemTest("c #{testCheat}", "New copy text")  # Check that we can read out


# Test too many arguments"
systemTest("c #{testCheat} \"Ordinary Text\" \"copy text\"", "Unexpected input : copy text")

# Test cheatsheet is number, which is not allowed
systemTest("c 1", "Error: Cheatsheet name can't not be an integer")

# Test trying to get a non-existing copy text
systemTest("c #{testCheat} 100", "Error: No copytext found for number:100")

#
# Cleanup
#
system("rm -f #{$testFile}")
