#!/usr/bin/env ruby
#
# Converts results of run_league.rb to CSV.

players = {}

Dir.glob('results/*').each do |f|
  r = File.read(f).split("\n")[-1]
  cpu1, cpu2 = f.sub('results/', '').split('-vs-').map{|_|_.sub('.sh', '')}
  win1, _, draw, _, win2, *_ = r.split(' ')

  players[cpu1] = {} if !players[cpu1]
  players[cpu1][cpu2] = win1
  players[cpu2] = {} if !players[cpu2]
  players[cpu2][cpu1] = win2
end

names = players.keys.sort
puts ['', names] * ','
names.each do |n|
  puts [n, names.map{|m|players[n][m]}] * ','
end
