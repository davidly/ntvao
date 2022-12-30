1 rem Apple 1 Basic version of app to prove you can't win at tic-tac-toe
30 dim b0(9), s1(10), s2(10), s4(10), s8(10), s6(10)
38 m1 = 0
39 gosub 1000
40 for l = 1 to 1
41 m1 = 0 : a1 = 2 : b1 = 9 : b0(1) = 1
45 gosub 4000
58 a1 = 2 : b1 = 9 : b0(1) = 0 : b0(2) = 1
62 gosub 4000
68 a1 = 2 : b1 = 9 : b0(2) = 0 : b0(5) = 1
72 gosub 4000
73 b0(5) = 0
80 next l
87 print "final move count (6493 expected): "; m1
88 print "$"
100 end

1000 for i = 1 to 9
1010 b0(i) = 0
1020 next i
1030 return

2000 w1 = b0(1)
2010 if 0 = w1 then 2100
2020 if w1 = b0(2) and w1 = b0(3) then return
2030 if w1 = b0(4) and w1 = b0(7) then return
2100 w1 = b0(4)
2110 if 0 = w1 then 2200
2120 if w1 = b0(5) and w1 = b0(6) then return
2200 w1 = b0(7)
2210 if 0 = w1 then 2300
2220 if w1 = b0(8) and w1 = b0(9) then return
2300 w1 = b0(2)
2310 if 0 = w1 then 2400
2320 if w1 = b0(5) and w1 = b0(8) then return
2400 w1 = b0(3)
2410 if 0 = w1 then 2500
2420 if w1 = b0(6) and w1 = b0(9) then return
2500 w1 = b0(5)
2510 if 0 = w1 then return
2520 if w1 = b0(1) and w1 = b0(9) then return
2530 if w1 = b0(3) and w1 = b0(7) then return
2540 w1 = 0
2550 return

4000 rem 
4030 d = 0 : i0 = 0 : v = 0 : r1 = 0
4100 m1 = m1 + 1
4102 if d < 4 then 4150
4103 gosub 2000
4105 if 0 = w1 then 4140
4110 if w1 = 1 then r1 = 6
4111 if w1 = 1 then 4280
4115 r1 = 4
4116 goto 4280
4140 if d = 8 then r1 = 5
4141 if d = 8 then 4280
4150 if i0 = 1 then v = 2
4151 if i0 = 1 then 4160
4155 v = 9
4160 p = 1
4180 if 0 # b0(p) then 4500
4200 if i0 = 1 then b0(p) = 1
4201 if i0 = 1 then 4215
4203 b0(p) = 2
4215 d1 = d + 1
4220 s1(d1) = p : s2(d1) = v : s4(d1) = a1 : s8(d1) = b1 : s6(d1) = i0
4248 if i0 = 0 then 4255
4249 i0 = 0
4250 goto 4260
4255 i0 = 1
4260 d = d + 1
4270 goto 4100
4280 d1 = d : d = d - 1
4285 i0 = s6(d1) : p = s1(d1) : v = s2(d1) : a1 = s4(d1) : b1 = s8(d1) : b0(p) = 0
4330 if i0 = 1 then 4340
4331 if r1 = 4 then 4530
4332 if r1 < v then v = r1
4334 if v < b1 then b1 = v
4336 if b1 <= a1 then 4520
4338 goto 4500
4340 if r1 = 6 then 4530
4341 if r1 > v then v = r1
4342 if v > a1 then a1 = v
4344 if a1 >= b1 then 4520
4500 p = p + 1
4505 if p < 10 then 4180
4520 r1 = v
4530 if d = 0 then return
4540 goto 4280

