#ifndef _GBROM_H
#define _GBROM_H

#include <Arduino.h>
#include "../gbConfig.h"

#include "rom/gbCart3_D_Crazy_Coaster_1983.h"
#include "rom/gbCart3_D_Mine_Storm_1983.h"
#include "rom/gbCart3_D_Narrow_Escape_1983.h"
#include "rom/gbCart8Ball_by_Christopher_Tumber_2001_PD.h"
#include "rom/gbCartAll_Good_Things_by_John_Dondzila_1996.h"
#include "rom/gbCartAnimaction_1983_light_pen.h"
#include "rom/gbCartArmor_Attack_1982.h"
#include "rom/gbCartArt_Master_1983_light_pen.h"
#include "rom/gbCartBattle_Earth_Terror_Hazard__Vecsports_Boxing_2000_PD.h"
#include "rom/gbCartBedlam_1983.h"
#include "rom/gbCartBerzerk_1982.h"
#include "rom/gbCartBirds_of_Prey_by_John_Dondzila_1999.h"
#include "rom/gbCartBlitz___Action_Football_1982.h"
#include "rom/gbCartBubble_Bobble_Diamond_Room_by_Chris_Salomon_1998_PD.h"
#include "rom/gbCartClean_Sweep_1982.h"
#include "rom/gbCartCosmic_Chasm_1982.h"
#include "rom/gbCartEngine_Analyzer_1983_light_pen.h"
#include "rom/gbCartEtch_a_Sketch_by_Jeff_Woolsey_1985_light_pen_PD.h"
#include "rom/gbCartFortress_of_Narzod_1982.h"
#include "rom/gbCartGravitrex_by_John_Dondzila_2002.h"
#include "rom/gbCartHeads_Up___Action_Soccer_1983.h"
#include "rom/gbCartHyperchase_1982.h"
#include "rom/gbCartLabyrinth_Rev_1_by_Chris_Salomon_1998_PD.h"
#include "rom/gbCartMelody_Master_1983_light_pen.h"
#include "rom/gbCartMine_Storm_1982.h"
#include "rom/gbCartMine_Storm_II_1983.h"
#include "rom/gbCartMissile_Lords_by_Christopher_Tumber_2001_PD.h"
#include "rom/gbCartNona3_by_Manu_Mar_10_2003.h"
#include "rom/gbCartOmega_Chase_by_Christopher_Tumber_2000_PD.h"
#include "rom/gbCartPatriots_by_John_Dondzila_1996.h"
#include "rom/gbCartPatriots_III___BALListics_Busters_by_John_Dondzila_1996.h"
#include "rom/gbCartPatriots_III___BALListics_Busters_by_John_Dondzila_2002.h"
#include "rom/gbCartPatriots_Remix_by_John_Dondzila_1999.h"
#include "rom/gbCartPolar_Rescue_1983.h"
#include "rom/gbCartPole_Position_1982.h"
#include "rom/gbCartRepulse_by_John_Dondzila_1999.h"
#include "rom/gbCartRip_Off_1982.h"
#include "rom/gbCartRockaroids_Remix___3rd_Rock_by_John_Dondzila_1996.h"
#include "rom/gbCartRonens_Game_Cart_2000_Ronen_Habot.h"
#include "rom/gbCartScramble_1982.h"
#include "rom/gbCartSolar_Quest_1982.h"
#include "rom/gbCartSpace_Wars_1982.h"
#include "rom/gbCartSpike_1983.h"
#include "rom/gbCartSpike_Goes_Skiing_1998_PD.h"
#include "rom/gbCartSpike_Hoppin_by_John_Dondzila_1998.h"
#include "rom/gbCartSpikes_Water_Balloons_Analog_by_John_Dondzila_2001_PD.h"
#include "rom/gbCartSpinball_1983.h"
#include "rom/gbCartStar_Castle_1983.h"
#include "rom/gbCartStar_Fire_Spirits_by_John_Dondzila_1999.h"
#include "rom/gbCartStar_Hawk_1982.h"
#include "rom/gbCartStar_Seige_by_John_Dondzila_1999.h"
#include "rom/gbCartStar_Ship_1982.h"
#include "rom/gbCartStar_Trek___The_Motion_Picture_1982.h"
#include "rom/gbCartTour_De_France_1983_Prototype.h"
#include "rom/gbCartTsunami_by_Christopher_Tumber_2001_PD.h"
#include "rom/gbCartVecmania_Part_1_1999_PD.h"
#include "rom/gbCartVecmania_Part_2_1999_PD.h"
#include "rom/gbCartVecSports_Kingpin_Bowling_by_Manu_V04_Aug_26_2002.h"
#include "rom/gbCartVectopia_by_John_Dondzila_2001_PD.h"
#include "rom/gbCartVector_Vaders_Remix_by_John_Dondzila_1999.h"
#include "rom/gbCartVectrace_2000_Ronen_Habot.h"
#include "rom/gbCartVectrexians_1999_PD.h"
#include "rom/gbCartVectrex_Maze_by_Chris_Salomon_1998_PD.h"
#include "rom/gbCartVectrex_Pong_1998_PD.h"
#include "rom/gbCartVersion_Nine_by_Christopher_Tumber_2000_PD.h"
#include "rom/gbCartVerzerk_by_Alex_Herbert_2002.h"
#include "rom/gbCartVexperience___BETH__Vecsports_Boxing_by_Manu_2000.h"
#include "rom/gbCartV_Frogger_by_Chris_Salomon_1998_PD.h"
#include "rom/gbCartWeb_Warp_1983.h"
#include "rom/gbCartWe_Wish_You_a_Merry_Christmas_by_J_Woolsey_1985_PD.h"
#include "rom/gbCartWormhole_by_John_Dondzila_2001_PD.h"

#define max_list_rom 71

static const char * gb_list_rom_title[max_list_rom] = {
    "3-D Crazy Coaster (1983).vec",
    "3-D Mine Storm (1983).vec",
    "3-D Narrow Escape (1983).vec",
    "8Ball by Christopher Tumber (2001) (PD).vec",
    "All Good Things by John Dondzila (1996).vec",
    "Animaction (1983) (light pen).vec",
    "Armor Attack (1982).vec",
    "Art Master (1983) (light pen).vec",
    "Battle Earth Terror Hazard & Vecsports Boxing (2000) (PD).vec",
    "Bedlam (1983).vec",
    "Berzerk (1982).vec",
    "Birds of Prey by John Dondzila (1999).vec",
    "Blitz! - Action Football (1982).vec",
    "Bubble Bobble (Diamond Room) by Chris Salomon (1998) (PD).vec",
    "Clean Sweep (1982).vec",
    "Cosmic Chasm (1982).vec",
    "Engine Analyzer (1983) (light pen).vec",
    "Etch-a-Sketch by Jeff Woolsey (1985) (light pen) (PD).vec",
    "Fortress of Narzod (1982).vec",
    "Gravitrex by John Dondzila (2002).vec",
    "Heads Up - Action Soccer (1983).vec",
    "Hyperchase (1982).vec",
    "Labyrinth Rev 1 by Chris Salomon (1998) (PD).vec",
    "Melody Master (1983) (light pen).vec",
    "Mine Storm (1982).vec",
    "Mine Storm II (1983).vec",
    "Missile Lords by Christopher Tumber (2001) (PD).vec",
    "Nona3 by Manu (Mar 10) (2003).vec",
    "Omega Chase by Christopher Tumber (2000) (PD).vec",
    "Patriots by John Dondzila (1996).vec",
    "Patriots III - BALListics Busters by John Dondzila (1996).vec",
    "Patriots III - BALListics Busters by John Dondzila (2002).vec",
    "Patriots Remix by John Dondzila (1999).vec",
    "Polar Rescue (1983).vec",
    "Pole Position (1982).vec",
    "Repulse by John Dondzila (1999).vec",
    "Rip-Off (1982).vec",
    "Rockaroids Remix - 3rd Rock by John Dondzila (1996).vec",
    "Ronen's Game Cart (2000) (Ronen Habot).vec",
    "Scramble (1982).vec",
    "Solar Quest (1982).vec",
    "Space Wars (1982).vec",
    "Spike (1983).vec",
    "Spike Goes Skiing (1998) (PD).vec",
    "Spike Hoppin' by John Dondzila (1998).vec",
    "Spike's Water Balloons (Analog) by John Dondzila (2001) (PD).vec",
    "Spinball (1983).vec",
    "Star Castle (1983).vec",
    "Star Fire Spirits by John Dondzila (1999).vec",
    "Star Hawk (1982).vec",
    "Star Seige by John Dondzila (1999).vec",
    "Star Ship (1982).vec",
    "Star Trek - The Motion Picture (1982).vec",
    "Tour De France (1983) (Prototype).vec",
    "Tsunami by Christopher Tumber (2001) (PD).vec",
    "Vecmania Part 1 (1999) (PD).vec",
    "Vecmania Part 2 (1999) (PD).vec",
    "VecSports Kingpin Bowling by Manu (V0.4) (Aug 26) (2002).vec",
    "Vectopia by John Dondzila (2001) (PD).vec",
    "Vector Vaders Remix by John Dondzila (1999).vec",
    "Vectrace (2000) (Ronen Habot).vec",
    "Vectrexians (1999) (PD).vec",
    "Vectrex Maze by Chris Salomon (1998) (PD).vec",
    "Vectrex Pong (1998) (PD).vec",
    "Version Nine by Christopher Tumber (2000) (PD).vec",
    "Verzerk by Alex Herbert (2002).vec",
    "Vexperience - B.E.T.H. & Vecsports Boxing by Manu (2000).vec",
    "V-Frogger by Chris Salomon (1998) (PD).vec",
    "Web Warp (1983).vec",
    "We Wish You a Merry Christmas by J. Woolsey (1985) (PD).vec",
    "Wormhole by John Dondzila (2001) (PD).vec"
};

static const unsigned int gb_list_cart_size[max_list_rom] = {
    8192,
    8192,
    8192,
    1805,
    16384,
    8192,
    4096,
    4096,
    3823,
    4096,
    4096,
    7156,
    8192,
    820,
    4096,
    4096,
    4096,
    512,
    8192,
    32768,
    8192,
    4096,
    3944,
    8192,
    8192,
    4096,
    8685,
    7436,
    16361,
    8169,
    2345,
    16384,
    8393,
    8192,
    8192,
    6073,
    4096,
    4245,
    15602,
    4096,
    4096,
    4096,
    8192,
    2010,
    16384,
    2980,
    8192,
    4096,
    8516,
    4096,
    7326,
    4096,
    4096,
    8192,
    11407,
    32768,
    32768,
    5783,
    32768,
    8103,
    3976,
    13434,
    626,
    1214,
    8387,
    29218,
    4148,
    16384,
    8192,
    914,
    32768
};

static const unsigned char * gb_list_rom_data[max_list_rom] = {
    gb_cart_3_D_Crazy_Coaster_1983,
    gb_cart_3_D_Mine_Storm_1983,
    gb_cart_3_D_Narrow_Escape_1983,
    gb_cart_8Ball_by_Christopher_Tumber_2001_PD,
    gb_cart_All_Good_Things_by_John_Dondzila_1996,
    gb_cart_Animaction_1983_light_pen,
    gb_cart_Armor_Attack_1982,
    gb_cart_Art_Master_1983_light_pen,
    gb_cart_Battle_Earth_Terror_Hazard__Vecsports_Boxing_2000_PD,
    gb_cart_Bedlam_1983,
    gb_cart_Berzerk_1982,
    gb_cart_Birds_of_Prey_by_John_Dondzila_1999,
    gb_cart_Blitz___Action_Football_1982,
    gb_cart_Bubble_Bobble_Diamond_Room_by_Chris_Salomon_1998_PD,
    gb_cart_Clean_Sweep_1982,
    gb_cart_Cosmic_Chasm_1982,
    gb_cart_Engine_Analyzer_1983_light_pen,
    gb_cart_Etch_a_Sketch_by_Jeff_Woolsey_1985_light_pen_PD,
    gb_cart_Fortress_of_Narzod_1982,
    gb_cart_Gravitrex_by_John_Dondzila_2002,
    gb_cart_Heads_Up___Action_Soccer_1983,
    gb_cart_Hyperchase_1982,
    gb_cart_Labyrinth_Rev_1_by_Chris_Salomon_1998_PD,
    gb_cart_Melody_Master_1983_light_pen,
    gb_cart_Mine_Storm_1982,
    gb_cart_Mine_Storm_II_1983,
    gb_cart_Missile_Lords_by_Christopher_Tumber_2001_PD,
    gb_cart_Nona3_by_Manu_Mar_10_2003,
    gb_cart_Omega_Chase_by_Christopher_Tumber_2000_PD,
    gb_cart_Patriots_by_John_Dondzila_1996,
    gb_cart_Patriots_III___BALListics_Busters_by_John_Dondzila_1996,
    gb_cart_Patriots_III___BALListics_Busters_by_John_Dondzila_2002,
    gb_cart_Patriots_Remix_by_John_Dondzila_1999,
    gb_cart_Polar_Rescue_1983,
    gb_cart_Pole_Position_1982,
    gb_cart_Repulse_by_John_Dondzila_1999,
    gb_cart_Rip_Off_1982,
    gb_cart_Rockaroids_Remix___3rd_Rock_by_John_Dondzila_1996,
    gb_cart_Ronens_Game_Cart_2000_Ronen_Habot,
    gb_cart_Scramble_1982,
    gb_cart_Solar_Quest_1982,
    gb_cart_Space_Wars_1982,
    gb_cart_Spike_1983,
    gb_cart_Spike_Goes_Skiing_1998_PD,
    gb_cart_Spike_Hoppin_by_John_Dondzila_1998,
    gb_cart_Spikes_Water_Balloons_Analog_by_John_Dondzila_2001_PD,
    gb_cart_Spinball_1983,
    gb_cart_Star_Castle_1983,
    gb_cart_Star_Fire_Spirits_by_John_Dondzila_1999,
    gb_cart_Star_Hawk_1982,
    gb_cart_Star_Seige_by_John_Dondzila_1999,
    gb_cart_Star_Ship_1982,
    gb_cart_Star_Trek___The_Motion_Picture_1982,
    gb_cart_Tour_De_France_1983_Prototype,
    gb_cart_Tsunami_by_Christopher_Tumber_2001_PD,
    gb_cart_Vecmania_Part_1_1999_PD,
    gb_cart_Vecmania_Part_2_1999_PD,
    gb_cart_VecSports_Kingpin_Bowling_by_Manu_V04_Aug_26_2002,
    gb_cart_Vectopia_by_John_Dondzila_2001_PD,
    gb_cart_Vector_Vaders_Remix_by_John_Dondzila_1999,
    gb_cart_Vectrace_2000_Ronen_Habot,
    gb_cart_Vectrexians_1999_PD,
    gb_cart_Vectrex_Maze_by_Chris_Salomon_1998_PD,
    gb_cart_Vectrex_Pong_1998_PD,
    gb_cart_Version_Nine_by_Christopher_Tumber_2000_PD,
    gb_cart_Verzerk_by_Alex_Herbert_2002,
    gb_cart_Vexperience___BETH__Vecsports_Boxing_by_Manu_2000,
    gb_cart_V_Frogger_by_Chris_Salomon_1998_PD,
    gb_cart_Web_Warp_1983,
    gb_cart_We_Wish_You_a_Merry_Christmas_by_J_Woolsey_1985_PD,
    gb_cart_Wormhole_by_John_Dondzila_2001_PD
};

#endif
