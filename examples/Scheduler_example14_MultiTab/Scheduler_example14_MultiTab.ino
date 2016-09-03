//This file is intentionally left blank.
//
//Arduino IDE plays some dirty tricks on the main sketch .ino file:
//it rearranges #includes, blindly creates forward definitions,
//includes every file in the project that does not have .c or .cpp
//file extension.
//
//Usually it all turns well if you have only one source file and you are either
//inexperienced or really expert C++ Arduino programmer.
//For the folks with the middle ground skills level, when you want
//to split your code into several .cpp files, it is best to leave
//this main sketch empty.
//
//It doesn't matter where you define the void loop() and void setup().
//Just make sure there is exactly one definition of each.
//
//And if you want to use standard Arduino functions
//like digitalWrite or the Serial object - just add #include<Arduino.h>.
