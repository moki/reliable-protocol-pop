# valid pop request
echo -ne '\x61\xC4\x01\x01\x00\x00\x00\x00\x01\x00\x00\x00If you are a musician today and you are not doing heroine I dont want to listen to your music' | nc -cu localhost 4000
# valid pop request
echo -ne '\x61\xC4\x01\x01\x00\x00\x00\x00\x01\x00\x00\x00Vitor Belfort still gets dizzy walking past Footlocker.' | nc -cu localhost 4000
# valid pop request
echo -ne '\x61\xC4\x01\x01\x00\x00\x00\x00\x01\x00\x00\x00Joey: I flew today later than Joe, and i had a little bag of riffa in my...Under my left nut.\
A.Jones: No we are not telling this story right now. Joey: YES WE ARE!YES!FOR FREEDOM OF SPEECH!I AM CALLIG OBAMA RIGHT NOW, MOTHERFUCKER!' | nc -cu localhost 4000
# valid pop request
echo -ne '\x61\xC4\x01\x01\x00\x00\x00\x00\x01\x00\x00\x00Aight Bye' | nc -cu localhost 4000
# invalid pop request
echo -ne '\x61\x00' | nc -cu localhost 4000
# shutdown server
echo -ne 'q' | nc -cu localhost 4000
