#!/usr/bin/env python3
"""Downscale ASCII art portrait from ~100x53 to 40x22."""

# The source ASCII art lines (without printf/quotes)
SOURCE_ART = r"""                          :yhhyyo-............`````.-://///://++osyyhhhhy.                          
                         /mNNNmh/-....`````````````..-://://++osyhdmNNNNNd-                         
                        +mNmNmh+-..```` `         ```..--.--:+ossyhdmmNNNNd:                        
                       `ymdmmms:..```              ````.``..-:+osyhdmmNNNNmd:                       
                      oNmmddd+-.```                ``````..-:+ssyhddmNNNNmmd-                      
                      `ymmdddh/..```                ``````..-:+oyhhhdmNNNNNmm-                      
                       +ddddhs:..```                ``````.-:/+oyhhhdmNNNNNmm/                      
                       :hhdhyo/-.```                 `````.:://+ssyhddmmmNNmm+                      
                       /yyhy++:-.``                  `````.---:/+oyhddmmdmNNms                      
                      `oyhhs+/:.```                     ````.--:/+yhddmmddNNNd`                     
                     ./yhhyo///.```                     ````..--:+syhdmmmdmNNd+-                    
                    `yhsymhy++:.```                     ````.-.-:/oshdmmNmmNNNNNs                   
                    -ddosydys+-``..```         ``````````.......-/osyhmmNNNNNMNNh                   
                    .hs++oydy:..../syo//:-.`````..``````.:/+++oooyhddmmmmNNNNNNmy                   
                     +osoohhy/.`.`..-/oshhyys+/:-.```-/oyhhhddddhyyhdmmmmNNNNNNd:                   
                     -sddhys:+-```.+ss:yhmdsho+:`````:hmmmmshdmddmmdhdmmmmNNNNNh                    
                     `/hh/++-./.```.````::::/:.-.` `-+ddmdy/:+o+oyddhhhddmmmmNNd                    
                      -+yo//-.....`    ```..-..::```./yyyho:...--/osooyhdmmdmNms                    
                      ./://+:.````      ```````.`  `./yyhyo/-....----+ohdmmdmmm:                    
                       -.-/o/-.``       ```````     `/yyss+:-.```..-:/ydmmmdmmd`                    
                       .--:o+/-.``````  ```````     `/shhyo/-....-::+shmmmmdmms                     
                        .-:/++/:.````` ``````````   `-shmmh/::---:+osymmNNmddy`                     
                         `.::++:-.````````````` ``  `/hhhdms/::::/oyhdmNmmmh+.                      
                          `-://:-.````   ```` ````  `:ydmmNds/::/+oydmNmmmmo                        
                           --:::-..`````````   ..```.omNMNNdyo+++shdmmmmmmm:                        
                           `:-::-...```````````.:+:+ohNMMNNdhhoosshmddmmdmd.                        
                            .:::-...`````.--.-:-/osyyyhmNmmmddddhydmdhhmmmy`                        
                             -::-.-.```.-/+//-/+oooyhyydmmmNNNNdhhydyhdmmm/                         
                              -:----...-/ss++/:/++++ooyhdmNNNmddosyyydmmmh`                         
                               .:--::..:+o-``````....-:/oyhdddhyyyhhhmmmm:                          
                                .:/+o+:/+-`````..:://o++oyhdddyoydmmmmdmm-                          
                                 :+ss+/+-.````.-/oosyyyyhdddhyoodNNmmdmNNd.                         
                                 -//++//:-.`````---:/+ooyhyossoomNNNNNMMMMmo-                       
                                 `o++o++:--..-.`.....:/+ooo//ohdmNNNMMMMMMMMNo-`                    
                                  +oooo+/-.`..---:/:++soo+ooohmNNNMMMMMMMMMMMMMdhyo+-`              
                                 `h+++oso/--.-/+/o+--/hyoosshNMMMMMMMMMMMMMMMMMMMMMMMds+:.``        
                                 .Ny+//+syy+///------:mmdddNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMNmhs/.`    
                              .:smNNds++/+osy:`````.-:sNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNdo:` 
                           `omNNNNNMMMmhs+//+/.```````./hMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN/
                       `/sdNNMMMNMNMMMMMMMNd+/:.``  ````.:yNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
                   `:ohNNNMMMMMMMNNMMMMMMMM/.```..`     `-/+hNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
                -+hmNMMMMMMMMNMNNMMMMMMMMMN.``````.` `   `/sodMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
             .+hmNMMMMMMMMMMNNNMNNNMMMMMMMM+`````````````-oyymNmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
         `-+hmMMMMMMMMMMMMMMMNNNNNNMMMMMMh/:.`````.--..-:+oydmNmhNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
     `-+ydNNMMMMMMMMMMMMMMMMMMNNNNNNNMNh-....``` .:+/://+oshdmNmmMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 `:ohmNNMMMMMMMMMMMMMMMMMMMMMMMNNNNNNNm+.``````..:++++ooosyhdmNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 -sdNMMMMMMMMMMMMMMMMMMMNNNMMMMNNNNNNNNd/.-```````.-/o+/syyssyhhdmNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 oMMMMMMMMMMMMMMMMMMMNNNNMNNNNNNNNNNNNN/```````.---:/+++osssssyhdmmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 oMMMMMMMMMMMMMMMMMMNNNNNNNNNNNNNNNNNNm.````..-/::::////++ossssyddmNNNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 oMMMMMMMMMMMMMMMMMMNNNNNNNNNNNNNNNNNNm-````.-://:///+//+++osssyhddmmNNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 oMMMMMMMMMMMMMMMMMMMMNNNNNNNMNMMMMMMN/..-::/+++///+++///+osssyyhddmmNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMo
 +hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhs:--::::::::/::::::///+++oossyyhhddddddddddhhhhhhhhhhhhhhhhhhh+"""

# Character to brightness mapping (0=lightest, higher=darker/denser)
CHAR_BRIGHTNESS = {}

# Space and very light chars
CHAR_BRIGHTNESS[' '] = 0
CHAR_BRIGHTNESS['`'] = 1
CHAR_BRIGHTNESS['.'] = 2
CHAR_BRIGHTNESS["'"] = 2
CHAR_BRIGHTNESS[','] = 2
CHAR_BRIGHTNESS['-'] = 3
CHAR_BRIGHTNESS[':'] = 4
CHAR_BRIGHTNESS[';'] = 4
CHAR_BRIGHTNESS['!'] = 5
CHAR_BRIGHTNESS['/'] = 5
CHAR_BRIGHTNESS['\\'] = 5
CHAR_BRIGHTNESS['|'] = 5
CHAR_BRIGHTNESS['+'] = 6
CHAR_BRIGHTNESS['='] = 6
CHAR_BRIGHTNESS['*'] = 7
CHAR_BRIGHTNESS['('] = 5
CHAR_BRIGHTNESS[')'] = 5
CHAR_BRIGHTNESS['['] = 5
CHAR_BRIGHTNESS[']'] = 5
CHAR_BRIGHTNESS['{'] = 5
CHAR_BRIGHTNESS['}'] = 5
CHAR_BRIGHTNESS['o'] = 8
CHAR_BRIGHTNESS['s'] = 8
CHAR_BRIGHTNESS['y'] = 9
CHAR_BRIGHTNESS['h'] = 10
CHAR_BRIGHTNESS['d'] = 11
CHAR_BRIGHTNESS['m'] = 12
CHAR_BRIGHTNESS['N'] = 13
CHAR_BRIGHTNESS['M'] = 14
CHAR_BRIGHTNESS['#'] = 14
CHAR_BRIGHTNESS['%'] = 14
CHAR_BRIGHTNESS['@'] = 15

# Fill in any other alphanumeric as medium-high density
for c in 'abcefgijklnpqrtuvwxzABCDEFGHIJKLOPQRSTUVWXYZ0123456789':
    if c not in CHAR_BRIGHTNESS:
        CHAR_BRIGHTNESS[c] = 10

# Output density ramp (light to dark)
RAMP = " .:-=+*#%@"

def get_brightness(ch):
    return CHAR_BRIGHTNESS.get(ch, 5)

def brightness_to_char(val):
    """Map brightness value to output character."""
    idx = int(val / 15.0 * (len(RAMP) - 1) + 0.5)
    idx = max(0, min(len(RAMP) - 1, idx))
    return RAMP[idx]

def main():
    lines = SOURCE_ART.split('\n')
    # Remove empty first/last lines if present
    while lines and lines[0].strip() == '':
        lines.pop(0)
    while lines and lines[-1].strip() == '':
        lines.pop()

    src_height = len(lines)
    src_width = max(len(l) for l in lines)

    # Pad all lines to same width
    padded = [l.ljust(src_width) for l in lines]

    target_w = 40
    target_h = 22

    print(f"// Source: {src_width}x{src_height} -> Target: {target_w}x{target_h}")
    print(f"// Block size: {src_width/target_w:.2f} x {src_height/target_h:.2f}")
    print()

    result = []
    for row in range(target_h):
        line = []
        # Calculate source row range
        y_start = row * src_height / target_h
        y_end = (row + 1) * src_height / target_h

        for col in range(target_w):
            # Calculate source col range
            x_start = col * src_width / target_w
            x_end = (col + 1) * src_width / target_w

            # Average brightness in this block
            total = 0.0
            count = 0

            # Iterate over all source pixels in this block
            for sy in range(int(y_start), min(int(y_end) + 1, src_height)):
                # Calculate vertical weight
                wy = 1.0
                if sy < y_start:
                    wy = 1.0 - (y_start - sy)
                elif sy + 1 > y_end:
                    wy = y_end - sy
                if wy <= 0:
                    continue

                for sx in range(int(x_start), min(int(x_end) + 1, src_width)):
                    # Calculate horizontal weight
                    wx = 1.0
                    if sx < x_start:
                        wx = 1.0 - (x_start - sx)
                    elif sx + 1 > x_end:
                        wx = x_end - sx
                    if wx <= 0:
                        continue

                    w = wx * wy
                    ch = padded[sy][sx] if sx < len(padded[sy]) else ' '
                    total += get_brightness(ch) * w
                    count += w

            avg = total / count if count > 0 else 0
            line.append(brightness_to_char(avg))

        result.append(''.join(line))

    # Print as C string literals
    print("// Downscaled portrait (40x22)")
    print("// Paste into C code:")
    print()
    for i, line in enumerate(result):
        # Escape backslashes and quotes
        escaped = line.replace('\\', '\\\\').replace('"', '\\"')
        print(f'    "{escaped}\\n"')

    print()
    print("// Raw preview:")
    for line in result:
        print(line)

if __name__ == '__main__':
    main()
