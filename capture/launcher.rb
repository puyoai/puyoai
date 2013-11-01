#!/usr/bin/env ruby
# -*- coding: utf-8 -*-

require 'fileutils'
require 'webrick'

FileUtils.mkdir_p('logs')

def my_system(s)
  puts "$ #{s}"
end

AIS = [
       ['test_lockit', 'aaa/rendaS2_5/rensa.exe'],
       ['hamaji ネクネク見るの', 'cpu/hamaji/lps.sh'],
       ['hamaji ネクネク見ないの', 'cpu/hamaji/lps-fast.sh'],
       ['shinyak', 'cpu/shinyak/run.sh'],
       ['pascal', 'cpu/pascal/run.sh'],
       ['ichikawa', 'cpu/ichikawa/run.sh'],
       ['hiroshimizuno', 'cpu/hiroshimizuno/run.sh'],
]

srv = WEBrick::HTTPServer.new({ :Port => 2424 })
Signal.trap(:INT){ srv.shutdown }

srv.mount_proc('/') do |req, res|
  if req.query['ai']
    my_system('killall connect')

    aii = req.query['ai'].to_i
    ai = AIS[aii]
    log = 'logs/%02d' % aii + Time.now.strftime('-%F-%X')
    opts = ["--puyofu_field_transition_log=#{log}.log"]
    my_system("(./capture/connect #{opts * ' '} /dev/video1 #{ai[1]} - ) 2> #{log}.err | ./capture/hidmon-mod/puyo")
  end

  body = %Q(<html>
<head>
<title>puyoai launcher</title>
</head>

<body>

<form>
<select name="ai">
)

  AIS.each_with_index do |kv, i|
    body += %Q(<option value="#{i}">#{kv[0]}</option>)
  end

body += %Q(
</select>
<input type="submit">
</form>

</body>

</html>
)

  res.body = body
end

srv.start
