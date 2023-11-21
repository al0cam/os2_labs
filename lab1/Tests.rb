require 'socket'

i = 3
portNum = 1000 + 6 * 10 + i**rand()
portNum = portNum.round()
p portNum
# @udpSock = UDPSocket.new
# @udpSock.bind('127.0.0.1', portNum)

# @udpSock2 = UDPSocket.new
# @udpSock2.bind('127.0.0.1', (portNum +1))

def sendMessage(text, portNumber, udpSock)
    udpSock.send text, 0, '127.0.0.1', portNumber
end

def recieveMessage(udpSock)
    begin
        diego = (udpSock.recvfrom_nonblock(100))[0]
    rescue => exception
        return false
    end
end

# sendMessage('lesgoooooooooooo')

# puts recieveMessage


#reading arguments from the command line
ARGV.each do|a|
    puts "Argument: #{a}"
end
arguments = ARGV.join(' ').split("@")
p arguments
p arguments.count

array = []
sockArray = []
bruv = []

arguments.count.times do |i|
    udpSock = UDPSocket.new
    udpSock.bind('127.0.0.1', (portNum+i))
    sockArray << udpSock
end


# TODO: provjerit kak se ponasa array kad se mijenja unutar procesa
arguments.count.times do |i|
    if i == 0
        array << fork do
            bruv << 3
            puts "#{i}: #{recieveMessage(sockArray[i])}"
            sendMessage("ruby #{i}", (portNum+1), sockArray[i])
        end
    else
        array << fork do
            p bruv
            sendMessage("ruby #{i}", (portNum), sockArray[i])
            puts "#{i}: #{recieveMessage(sockArray[i])}"
        end
    end
end

sendingString = "z" +"(1, 1)"

puts sendingString
p sendingString.tr("()",'').tr('zoi','').split(',')[1].to_i

if 1 > 1 then p 'ye'
else p 'nah dawg'
end

Process.waitall







