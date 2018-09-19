{\rtf1\ansi\ansicpg1252\cocoartf1561\cocoasubrtf600
{\fonttbl\f0\fmodern\fcharset0 CourierNewPSMT;\f1\froman\fcharset0 Times-Roman;}
{\colortbl;\red255\green255\blue255;\red0\green0\blue0;\red255\green255\blue255;}
{\*\expandedcolortbl;;\cssrgb\c0\c0\c0;\cssrgb\c100000\c100000\c100000;}
\margl1440\margr1440\vieww10800\viewh8400\viewkind0
\deftab720
\pard\pardeftab720\sl280\partightenfactor0

\f0\fs24 \cf2 \cb3 \expnd0\expndtw0\kerning0
;This program counts from 10 to 0\
       .ORIG x3000\
\pard\pardeftab720\sl280\sa240\partightenfactor0
\cf2 LEA R0, TEN 
\f1 \cb1 \

\f0 \cb3 LDW R1, R0, #0 START ADD R1, R1, #-1 
\f1 \cb1 \
\pard\pardeftab720\sl280\partightenfactor0

\f0 \cf2 \cb3        BRZ DONE\
       BR START\
\pard\pardeftab720\sl280\sa240\partightenfactor0
\cf2 DONE TRAP x25 TEN .FILL x000A 
\f1 \cb1 \

\f0 \cb3 .END 
\f1 \cb1 \
\pard\pardeftab720\sl280\partightenfactor0

\f0 \cf2 \cb3 ;This instruction will be loaded into memory location x3000\
;blank line\
;The last executable instruction\
;This is 10 in 2's comp, hexadecimal\
;The pseudo-op, delimiting the source program\
}