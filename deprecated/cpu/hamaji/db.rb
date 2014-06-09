#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

class DB
  def initialize
    @smiles = []
    @date_to_smile = {}
    @smile_to_index = {}
    name = nil
    player1 = nil
    player2 = nil
    date = nil
    index = 0
    File.readlines('urls.txt').each do |line|
      if /^(\S+) - (\S+) (\d+-\d+-..)$/ =~ line
        name = $&
        player1 = $1
        player2 = $2
        date = $3
      elsif /^http:\/\/.*?\/(sm\d+)$/ =~ line
        smile = $1
        @date_to_smile[date] = [] if !@date_to_smile[date]
        @date_to_smile[date] << smile

        @smile_to_index[smile] = index

        @smiles << {
          'name' => name,
          'player1' => player1,
          'player2' => player2,
          'date' => date,
          'smile' => smile,
        }

        index += 1
      elsif smile = misuken_avi_to_smile(line)
        @date_to_smile[date] = [] if !@date_to_smile[date]
        @date_to_smile[date] << smile

        @smile_to_index[smile] = index

        @smiles << {
          'name' => name,
          'player1' => player1,
          'player2' => player2,
          'date' => date,
          'smile' => smile,
        }

        index += 1
      end
    end
  end

  def misuken_avi_to_smile(avi)
    if /dat\/puyo\/Misuken vs Hattori_(\d+).avi$/ !~ avi
      return false
    end
    "misuken_hattori_#$1"
  end

  def mp4_to_smile(mp4)
    raise mp4 unless /(\d+_\d+_\d+).*?part(\d+)/i =~ mp4
    date = $1.tr('_', '-')
    part = $2.to_i
    @date_to_smile[date][part-1]
  end

  def smile_to_index(smile)
    @smile_to_index[smile]
  end

  def smiles
    @smiles
  end

  def add_title(dir)
    Dir.glob("#{dir}/*.mp4").each do |mp4|
      title = File.basename(mp4.sub(/.mp4/, '')).tr('_', '/')
      smile = mp4_to_smile(mp4)
      @smiles[@smile_to_index[smile]]['title'] = title
    end
    Dir.glob("#{dir}/*.avi").each do |avi|
      part = avi[/_(\d+)/, 1]
      title = "2004-05-?? ぷよぷよ通 ミスケン vs くまちょむ part#{part}"
      smile = misuken_avi_to_smile(avi)
      @smiles[@smile_to_index[smile]]['title'] = title
    end
  end
end
