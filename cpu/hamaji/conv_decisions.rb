#!/usr/bin/env ruby

require 'json'
require 'optparse'

require 'db'

MP4_DIR = "#{ENV['HOME']}/dat/puyo"

opts = OptionParser.new
$for_url = false
$for_db = true
opts.on('-u'){ $for_url = true; $for_db = false }
opts.parse!(ARGV)

TBL = [*'0'..'9'] + [*'a'..'z'] + [*'A'..'Z']

def enc_tumo(t)
  a = t[0].ord - ?A.ord
  b = t[1].ord - ?A.ord
  v = a * 2 + b * 12
  TBL[v]
end

def enc_dec(d)
  a = d[0].ord - ?1.ord
  b = "URDL".index(d[1])
  v = a * 2 + b * 12
  TBL[v]
end

out = []

db = DB.new
db.add_title(MP4_DIR)

ARGV.each do |arg|
  player = /_2p/ =~ arg ? 2 : 1
  smile = File.basename(arg).sub(/\..*/, '').sub(/_2p/, '')
  match_num = 0
  File.readlines(arg).each do |line|
    match_num += 1

    if line =~ /^\s*$/
      next
    end

    tumo, dec = line.split
    raise line if tumo.size != dec.size
    sz = tumo.size / 3

    if $for_url
      enc = 'http://ips.karou.jp/simu/pe.html?_'
      sz.times{|t|
        enc += enc_tumo(tumo[t*3,2]) + enc_dec(dec[t*3,2])
      }
      puts enc
    else
      tumos = ''
      decs = ''
      sz.times{|t|
        tumos += enc_tumo(tumo[t*3,2])
        decs += enc_dec(dec[t*3,2])
      }
      out << "'#{tumos} #{decs} #{db.smile_to_index(smile)} #{match_num} #{player}'"
    end
  end
end

if $for_db
  puts 'SMILES = ' + JSON.generate(db.smiles) + ';'
  puts 'DB = ['
  puts out.sort.uniq * ",\n"
  puts "];"
end
