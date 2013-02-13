#!/usr/bin/env ruby

NUM_WIN = 20

CPUS = %w(
  cpu/hamaji/lps.sh
  cpu/hamaji/lps-fast.sh
  cpu/hamaji/lps-gtr.sh
  cpu/hamaji/lps-gtr2.sh
  cpu/ichikawa/run.sh
  cpu/shinyak/run.sh
  cpu/pascal/run.sh
)

ENV['PUYO_GUI'] = '0'
ENV['PUYO_NUM_WIN'] = NUM_WIN.to_s

system("mkdir -p results")

CPUS.size.times{|i|
  (i+1).upto(CPUS.size-1){|j|
    cpu1 = CPUS[i]
    cpu2 = CPUS[j]
    result = `duel/duel #{cpu1} #{cpu2}`

    log = 'results/' + cpu1.tr('/', '_') + '_vs_' + cpu2.tr('/', '_')
    File.open(log, "w") do |of|
      of.puts result.split("\n").grep(/.* \/ .*/).join("\n")
    end
  }
}
