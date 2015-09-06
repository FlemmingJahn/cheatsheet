#!/usr/bin/env ruby
#
# Testcase for testing cheatsheet script
#

require 'pry'      # For debugging

def systemTest cmd, expectedResult
  puts cmd
  result = `#{cmd}`
  puts "Error: result:#{result}, expected result:#{expectedResult}" if result.match(expectedResult).nil?
end



systemTest("c newFile", "New cheatSheet named newFile created") # Testing creating new cheatsheet
puts "Error: New cheatSheet not created" if !File.exist?('newFile.txt')
system("rm newFile.txt")



# Create and insert cheat text
systemTest("c newFile \"Ordinary Text\"", "")  # Create new cheatSheet
puts "Error: New cheatSheet not created" if !File.exist?('newFile.txt')
systemTest("c newFile", "Ordinary Text")  # Check that we can read out the text

# Insert Header
systemTest("c newFile -H \"New Header\"", "")  #
systemTest("c newFile", "1 New Header")  # Check that we can read out the header


# Test too many arguments"
systemTest("c newFile \"Ordinary Text\" \"copy text\"", "Unexpected input : copy text")
#
# Cleanup
#
system("rm newFile.txt")
