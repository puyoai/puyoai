#!/usr/bin/env ruby
#
# A naive AI league runner

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

def normalize_ai_name(n)
  n.sub('cpu/', '').tr('/', '_')
end

CPUS.size.times{|i|
  (i+1).upto(CPUS.size-1){|j|
    cpu1 = CPUS[i]
    cpu2 = CPUS[j]
    log = ('results/' + normalize_ai_name(cpu1) +
           '-vs-' + normalize_ai_name(cpu2))
    puts log
    result = `duel/duel #{cpu1} #{cpu2}`

    File.open(log, "w") do |of|
      of.puts result.split("\n").grep(/.* \/ .*/).join("\n")
    end
  }
}
