#!/usr/bin/env ruby

require 'db'
#require 'json'

$is_2p = true

db = DB.new

ARGV.each do |arg|
  if /\.avi$/ =~ arg
    smile = db.misuken_avi_to_smile(arg)
  else
    smile = db.mp4_to_smile(arg)
  end

  puts "#{arg} => #{smile}"
  if $is_2p
    out = "data/#{smile}_2p.txt"
  else
    out = "data/#{smile}.txt"
  end
  if !File.exist?(out)
    cmd = ["./parse_movie",
           "--out=#{out}",
           "--show_video=0"]
    if $is_2p
      cmd << '--is_2p'
    end
    cmd << arg
    system(*cmd)
  end
end
