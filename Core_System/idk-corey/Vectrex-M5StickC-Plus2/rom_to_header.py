#!/usr/bin/env python3
"""Convert Vectrex ROM files to C header files for embedding"""

import os
import sys

def rom_to_header(rom_path, output_dir):
    """Convert a single ROM file to a C header"""
    rom_name = os.path.basename(rom_path)
    rom_name_safe = rom_name.replace(' ', '_').replace('-', '_').replace('(', '').replace(')', '').replace('.vec', '')
    rom_name_safe = ''.join(c for c in rom_name_safe if c.isalnum() or c == '_')
    
    with open(rom_path, 'rb') as f:
        rom_data = f.read()
    
    header_name = f"gbCart{rom_name_safe}.h"
    header_path = os.path.join(output_dir, header_name)
    
    with open(header_path, 'w') as f:
        f.write(f"// Auto-generated from {rom_name}\n")
        f.write(f"#ifndef _GBCART_{rom_name_safe.upper()}_H\n")
        f.write(f"#define _GBCART_{rom_name_safe.upper()}_H\n\n")
        f.write(f"const unsigned char gb_cart_{rom_name_safe}[] PROGMEM = {{\n")
        
        for i in range(0, len(rom_data), 16):
            chunk = rom_data[i:i+16]
            hex_values = ', '.join(f'0x{b:02X}' for b in chunk)
            f.write(f"    {hex_values},\n")
        
        f.write("};\n\n")
        f.write(f"#endif\n")
    
    return rom_name_safe, len(rom_data), rom_name

if __name__ == '__main__':
    roms_dir = sys.argv[1] if len(sys.argv) > 1 else '../roms'
    output_dir = sys.argv[2] if len(sys.argv) > 2 else '../src/dataFlash/rom'
    
    # List of ALL ROMs
    selected_roms = [
        '3-D Crazy Coaster (1983).vec',
        '3-D Mine Storm (1983).vec',
        '3-D Narrow Escape (1983).vec',
        '8Ball by Christopher Tumber (2001) (PD).vec',
        'All Good Things by John Dondzila (1996).vec',
        'Animaction (1983) (light pen).vec',
        'Armor Attack (1982).vec',
        'Art Master (1983) (light pen).vec',
        'Battle Earth Terror Hazard & Vecsports Boxing (2000) (PD).vec',
        'Bedlam (1983).vec',
        'Berzerk (1982).vec',
        'Birds of Prey by John Dondzila (1999).vec',
        'Blitz! - Action Football (1982).vec',
        'Bubble Bobble (Diamond Room) by Chris Salomon (1998) (PD).vec',
        'Clean Sweep (1982).vec',
        'Cosmic Chasm (1982).vec',
        'Engine Analyzer (1983) (light pen).vec',
        'Etch-a-Sketch by Jeff Woolsey (1985) (light pen) (PD).vec',
        'Fortress of Narzod (1982).vec',
        'Gravitrex by John Dondzila (2002).vec',
        'Heads Up - Action Soccer (1983).vec',
        'Hyperchase (1982).vec',
        'Labyrinth Rev 1 by Chris Salomon (1998) (PD).vec',
        'Melody Master (1983) (light pen).vec',
        'Mine Storm (1982).vec',
        'Mine Storm II (1983).vec',
        'Missile Lords by Christopher Tumber (2001) (PD).vec',
        'Nona3 by Manu (Mar 10) (2003).vec',
        'Omega Chase by Christopher Tumber (2000) (PD).vec',
        'Patriots by John Dondzila (1996).vec',
        'Patriots III - BALListics Busters by John Dondzila (1996).vec',
        'Patriots III - BALListics Busters by John Dondzila (2002).vec',
        'Patriots Remix by John Dondzila (1999).vec',
        'Polar Rescue (1983).vec',
        'Pole Position (1982).vec',
        'Repulse by John Dondzila (1999).vec',
        'Rip-Off (1982).vec',
        'Rockaroids Remix - 3rd Rock by John Dondzila (1996).vec',
        'Ronen\'s Game Cart (2000) (Ronen Habot).vec',
        'Scramble (1982).vec',
        'Solar Quest (1982).vec',
        'Space Wars (1982).vec',
        'Spike (1983).vec',
        'Spike Goes Skiing (1998) (PD).vec',
        'Spike Hoppin\' by John Dondzila (1998).vec',
        'Spike\'s Water Balloons (Analog) by John Dondzila (2001) (PD).vec',
        'Spinball (1983).vec',
        'Star Castle (1983).vec',
        'Star Fire Spirits by John Dondzila (1999).vec',
        'Star Hawk (1982).vec',
        'Star Seige by John Dondzila (1999).vec',
        'Star Ship (1982).vec',
        'Star Trek - The Motion Picture (1982).vec',
        'Tour De France (1983) (Prototype).vec',
        'Tsunami by Christopher Tumber (2001) (PD).vec',
        'Vecmania Part 1 (1999) (PD).vec',
        'Vecmania Part 2 (1999) (PD).vec',
        'VecSports Kingpin Bowling by Manu (V0.4) (Aug 26) (2002).vec',
        'Vectopia by John Dondzila (2001) (PD).vec',
        'Vector Vaders Remix by John Dondzila (1999).vec',
        'Vectrace (2000) (Ronen Habot).vec',
        'Vectrexians (1999) (PD).vec',
        'Vectrex Maze by Chris Salomon (1998) (PD).vec',
        'Vectrex Pong (1998) (PD).vec',
        'Version Nine by Christopher Tumber (2000) (PD).vec',
        'Verzerk by Alex Herbert (2002).vec',
        'Vexperience - B.E.T.H. & Vecsports Boxing by Manu (2000).vec',
        'V-Frogger by Chris Salomon (1998) (PD).vec',
        'Web Warp (1983).vec',
        'We Wish You a Merry Christmas by J. Woolsey (1985) (PD).vec',
        'Wormhole by John Dondzila (2001) (PD).vec',
    ]
    
    os.makedirs(output_dir, exist_ok=True)
    
    roms_info = []
    for rom_file in selected_roms:
        rom_path = os.path.join(roms_dir, rom_file)
        if os.path.exists(rom_path):
            print(f"Converting {rom_file}...")
            safe_name, size, display_name = rom_to_header(rom_path, output_dir)
            roms_info.append((safe_name, size, display_name))
        else:
            print(f"Warning: {rom_file} not found")
    
    # Generate gbrom.h
    gbrom_path = os.path.join(os.path.dirname(output_dir), 'gbrom.h')
    with open(gbrom_path, 'w') as f:
        f.write("#ifndef _GBROM_H\n")
        f.write("#define _GBROM_H\n\n")
        f.write("#include \"../gbConfig.h\"\n\n")
        
        # Include all ROM headers
        for safe_name, _, _ in roms_info:
            f.write(f"#include \"rom/gbCart{safe_name}.h\"\n")
        
        f.write(f"\n#define max_list_rom {len(roms_info)}\n\n")
        
        # ROM titles array
        f.write("static const char * gb_list_rom_title[max_list_rom] = {\n")
        for i, (_, _, display_name) in enumerate(roms_info):
            comma = "," if i < len(roms_info) - 1 else ""
            f.write(f'    "{display_name}"{comma}\n')
        f.write("};\n\n")
        
        # ROM sizes array
        f.write("static const unsigned int gb_list_cart_size[max_list_rom] = {\n")
        for i, (_, size, _) in enumerate(roms_info):
            comma = "," if i < len(roms_info) - 1 else ""
            f.write(f"    {size}{comma}\n")
        f.write("};\n\n")
        
        # ROM data pointers
        f.write("static const unsigned char * gb_list_rom_data[max_list_rom] = {\n")
        for i, (safe_name, _, _) in enumerate(roms_info):
            comma = "," if i < len(roms_info) - 1 else ""
            f.write(f"    gb_cart_{safe_name}{comma}\n")
        f.write("};\n\n")
        
        f.write("#endif\n")
    
    print(f"\nGenerated {len(roms_info)} ROM headers")
    print(f"Total size: {sum(size for _, size, _ in roms_info)} bytes")
