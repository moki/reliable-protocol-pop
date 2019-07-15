# HELLO pop packet
echo -ne '\x61\xC4\x01\x00\x00\x00\x00\x00\x01\x00\x00\x00' | nc -cu -p 8900 localhost 4000
# DATA pop packet
echo -ne '\x61\xC4\x01\x01\x01\x00\x00\x00\x01\x00\x00\x00If you are a musician today and you are not doing heroine I dont want to listen to your music' | nc -cu -p 8900 localhost 4000
# DATA pop packet
echo -ne '\x61\xC4\x01\x01\x02\x00\x00\x00\x01\x00\x00\x00Vitor Belfort still gets dizzy walking past Footlocker.' | nc -cu -p 8900 localhost 4000
# GOODBYE pop packet
echo -ne '\x61\xC4\x01\x03\x03\x00\x00\x00\x01\x00\x00\x00' | nc -cu -p 8901 localhost 4000
# HELLO pop packet
echo -ne '\x61\xC4\x01\x00\x00\x00\x00\x00\x02\x00\x00\x00' | nc -cu -p 8901 localhost 4000
# DATA pop packet
echo -ne '\x61\xC4\x01\x01\x01\x00\x00\x00\x02\x00\x00\x00Joey: I flew today later than Joe, and i had a little bag of riffa in my...Under my left nut.
A.Jones: No we are not telling this story right now. Joey: YES WE ARE!YES!FOR FREEDOM OF SPEECH!I AM CALLIG OBAMA RIGHT NOW, MOTHERFUCKER!' | nc -cu -p 8901 localhost 4000
# DATA pop packet
echo -ne '\x61\xC4\x01\x01\x02\x00\x00\x00\x02\x00\x00\x00Ladida h4HaHA' | nc -cu -p 8901 localhost 4000
# GOODBYE pop packet
echo -ne '\x61\xC4\x01\x03\x03\x00\x00\x00\x02\x00\x00\x00' | nc -cu -p 8901 localhost 4000
# invalid pop packet
echo -ne '\x61\x00' | nc -cu -p 8902 localhost 4000
# invalid pop packet
echo -ne 'q' | nc -cu -p 8902 localhost 4000
# invalid pop packet
echo -ne '\x61\x00' | nc -cu -p 8902 localhost 4000
# invalid pop packet
echo -ne 'q' | nc -cu -p 8902 localhost 4000
# invalid pop packet
echo -ne '\x61\x00' | nc -cu -p 8902 localhost 4000
# invalid pop packet
echo -ne 'q' | nc -cu -p 8902 localhost 4000
