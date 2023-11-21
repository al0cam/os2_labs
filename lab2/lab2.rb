require 'open3'

file = File.open('numbers.txt')
data = file.read
# puts data
numbers = data.split("\n")
Open3.popen2e("bc -q") do |i,o,t|
    numbers.length.times do |j|
        puts "#{numbers[j]}"
        i.puts numbers[j]
        value = gets.chomp
        piped = o.gets
        if piped.include?("error")
            puts "NEISPRAVAN IZRAZ"
        elsif piped.tr("\n",'') != value
            puts "NEISPRAVNO, točan odgovor je #{piped}"
        else
            puts "ISPRAVNO"
        end
    end
end