import random
import sys
import string

def main():
    if (len(sys.argv) != 3):
        print('incorrect usage. usage: genText.py [length] [filename]')
    length = int(sys.argv[1])
    out = str(sys.argv[2])
    fout = open(out, 'w')
    letters = 'abcdefghijklmnopqrstuvwxyz'

    i = 0
    while (i < length):
        runLen = random.randint(1, 15)
        if (length - i < runLen):
            runLen = length - i
        runChar = random.choice(letters)
        run = runChar * runLen
        fout.write(run)
        i += runLen

main()
