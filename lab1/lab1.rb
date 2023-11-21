require 'socket'
require 'pry'


def sendMessage(text, udpSock, portNumber)
    udpSock.send text, 0, '127.0.0.1', portNumber
end

## alternate version of recieve message which returns false on no message found else it returns a message
def recieveMessage(udpSock)
    begin
        diego = (udpSock.recvfrom_nonblock(100))[0]
    rescue => exception
        return false
    end
end


@portNum = 1000 + 6 * 10



arguments = ARGV.join(' ').split("@")

# getting the process count out of the input arguments
@processCount = arguments[0].chomp.split(" ").count



clockInitValues = []
clockInitValues = arguments[0].chomp.split(" ")

# array for storing where the process wants to enter the critical section
@requestPoints = []
@processCount.times do |i|
    if arguments[i+1] != nil
        @requestPoints << arguments[i+1].chomp.split(" ")
    else
        @requestPoints << ''
    end
end

array = []
@sockArray = []

# create udp sockets for communication between processes
@processCount.times do |i|
    udpSock = UDPSocket.new
    udpSock.bind('127.0.0.1', (@portNum+i))
    @sockArray << udpSock
end

## messageType can be request(z) or exit(i)
def sendToEveryone(messageType)
    if messageType.downcase == 'z' ||  messageType.downcase == 'i'
        sendingString = ''
        @processCount.times do |i|
            if i != @processNumber
                sendingString = messageType +"(#{@processNumber}, #{@clock})"
                puts "P(#{@processNumber}) poslao #{sendingString} to #{i}"
                sendMessage(sendingString, @sockArray[@processNumber], (@portNum + i))
            end
        end
        if messageType == 'z'
            @queue << sendingString
            sortQueue()
        end
    end
end

def sendResponse(senderProcessNumber)
    sendingString = "o" +"(#{@processNumber}, #{@clock})"
    puts "P(#{@processNumber}) poslao #{sendingString} to #{senderProcessNumber}"
    sendMessage(sendingString, @sockArray[@processNumber], (@portNum + senderProcessNumber))
end



def setClock(message)
    recievedClock = (message.tr("()",'').tr('zoi','').split(',')[1]).to_i
    if @clock > recievedClock 
        @clock+=1
    else 
        @clock = recievedClock + 1
    end
end

def processNumberFromMessage(message)
    message.tr("()",'').tr('zoi','').split(',')[0].to_i
end

def sortQueue()
    (@queue.length - 1).times do |value|
        currentValue = @queue[value].tr("()",'').tr('zoi','').split(',')[1].to_i
        nextValue = @queue[value + 1].tr("()",'').tr('zoi','').split(',')[1].to_i
        if currentValue > nextValue
            help = @queue[value]
            @queue[value] = @queue[value + 1]
            @queue[value + 1] = help
        elsif currentValue == nextValue and processNumberFromMessage(@queue[value]) > processNumberFromMessage(@queue[value + 1])
            help = @queue[value]
            @queue[value] = @queue[value + 1]
            @queue[value + 1] = help
        end
    end
end

def criticalSection()
    sendToEveryone('z')
    responseCount = 0
    loop do
        message = recieveMessage(@sockArray[@processNumber])
        if message
            puts "P(#{@processNumber}) primio #{message} od #{processNumberFromMessage(message)}"
        end
        if message and message.include?('z')
            setClock(message)
            @queue << message
            sortQueue()
            sendResponse(processNumberFromMessage(message))
        elsif message and message.include?('i')
            if @queue.length > 0 
                what = @queue.shift
            end
        elsif message and message.include?('o')
            setClock(message)
            responseCount += 1
        end 

        if responseCount == (@processCount - 1) and @queue[0]
            if @processNumber == processNumberFromMessage(@queue[0])
                @queue.shift
                break
            end
        end
    end
    puts "P(#{@processNumber}) USAO u KO +++++++++++++++++++++++++++++++++++++++++"
    sleep(3)
    puts "P(#{@processNumber}) IZASAO iz KO -----------------------------------------"
    sendToEveryone('i')
end

def job()
    sleep(3)
    while true
        puts "T(#{@processNumber})= #{@clock}"
        while @requestPoints[@processNumber].include?(@clock.to_s) 
            criticalSection()
        end
        message = recieveMessage(@sockArray[@processNumber])
        if message
            puts "P(#{@processNumber}) primio #{message} od #{processNumberFromMessage(message)}"
        end
        sleep(1)
        @clock+=1
        if message and message.include?('z')
            setClock(message)
            @queue << message
            sortQueue()
            sendResponse(processNumberFromMessage(message))
        elsif message and message.include?('i')
            if @queue.length > 0 
                @queue.shift
            end
        end
    end
end

@queue = []
@clock = 0
@processNumber
@processCount.times do |i|
    fork do
        @clock = clockInitValues[i].to_i
        @processNumber = i
        job()
    end
end

Process.waitall
