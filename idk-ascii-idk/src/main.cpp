#include <M5Unified.h>
#include "engine/ASCIIRenderer.h"
#include "engine/FrameTimer.h"
#include "menu/MenuSystem.h"
#include "animations/ascii2d/MatrixRain.h"
#include "animations/ascii2d/AsciiFire.h"
#include "animations/ascii2d/GameOfLife.h"
#include "animations/ascii3d/Torus3D.h"
#include "animations/physics_atom/BohrAtom.h"
#include "animations/physics_atom/WaveInterference.h"
#include "animations/chaos/LorenzAttractor.h"
#include "animations/nbody/Boids.h"
#include "animations/fractals/Mandelbrot.h"
#include "animations/ascii2d/Extra2D.h"
#include "animations/ascii3d/Cube3D.h"
#include "animations/chaos/DoublePendulum.h"
#include "animations/ascii2d/More2D.h"
#include "animations/ascii2d/Remaining2D.h"
#include "animations/ascii3d/More3D.h"
#include "animations/ascii3d/Remaining3D.h"
#include "animations/screensaver/Starfield.h"
#include "animations/fluid/FluidWave.h"
#include "animations/space/BlackHole.h"
#include "animations/math_geo/MathGeo.h"
#include "animations/RemainingMisc.h"
#include "animations/RemainingFractalChaos.h"
#include "animations/FinalBatch.h"
#include "animations/PlaceholderAnim.h"
#include "animations/FinalBatchGen.h"

using namespace idk;

ASCIIRenderer renderer;
FrameTimer timer(30);
MenuSystem menu;
IAnimation* activeAnim = nullptr;

// Simple factory to create animations
void startAnimation(int cat, int anim) {
    if (activeAnim) delete activeAnim;
    activeAnim = nullptr;

    switch (cat) {
        case 0: // ASCII 2D
            if (anim == 0) activeAnim = new MatrixRain();
            else if (anim == 1) activeAnim = new SineWave();
            else if (anim == 2) activeAnim = new AsciiFire();
            else if (anim == 3) activeAnim = new Fireworks();
            else if (anim == 4) activeAnim = new GameOfLife();
            else if (anim == 5) activeAnim = new MazeGen();
            else if (anim == 6) activeAnim = new WormSnake();
            else if (anim == 7) activeAnim = new WindParticles();
            else if (anim == 8) activeAnim = new Ripples();
            else if (anim == 9) activeAnim = new Typewriter();
            else if (anim == 10) activeAnim = new BouncingBall();
            else if (anim == 11) activeAnim = new LangtonsAnt();
            else if (anim == 12) activeAnim = new BinaryRain();
            else if (anim == 13) activeAnim = new RadarSweep();
            else if (anim == 14) activeAnim = new Spinner();
            else activeAnim = new PlaceholderAnim();
            break;
        case 1: // ASCII 3D
            if (anim == 0) activeAnim = new Torus3D();
            else if (anim == 1) activeAnim = new Cube3D();
            else if (anim == 2) activeAnim = new Sphere3D();
            else if (anim == 3) activeAnim = new InfiniteTunnel();
            else if (anim == 4) activeAnim = new DnaHelix();
            else if (anim == 5) activeAnim = new Pyramid3D();
            else if (anim == 7) activeAnim = new Wormhole();
            else if (anim == 8) activeAnim = new Hypercube();
                        else if (anim == 6) activeAnim = new MobiusStrip();
            else if (anim == 9) activeAnim = new FractalTree();
else activeAnim = new PlaceholderAnim();
            break;
        case 2: // Screensaver
            if (anim == 0) activeAnim = new MeteorShower();
            else if (anim == 8) activeAnim = new Starfield();
                        else if (anim == 1) activeAnim = new Aurora();
            else if (anim == 2) activeAnim = new Bubbles();
            else if (anim == 3) activeAnim = new SnowFall();
            else if (anim == 4) activeAnim = new OceanWaves();
            else if (anim == 5) activeAnim = new AutumnLeaves();
            else if (anim == 6) activeAnim = new Fireflies();
            else if (anim == 7) activeAnim = new SandStorm();
            else if (anim == 9) activeAnim = new Clock();
else activeAnim = new PlaceholderAnim();
            break;
        case 3: // Atom/Physics
            if (anim == 0) activeAnim = new BohrAtom();
            else if (anim == 4) activeAnim = new WaveInterference();
            else if (anim == 6) activeAnim = new PlasmaSim();
            else if (anim == 7) activeAnim = new RadioactiveDecay();
                        else if (anim == 1) activeAnim = new ElectronOrbitals();
            else if (anim == 2) activeAnim = new NuclearFission();
            else if (anim == 3) activeAnim = new EMWave();
            else if (anim == 5) activeAnim = new Photoelectric();
            else if (anim == 8) activeAnim = new Tunneling();
else activeAnim = new PlaceholderAnim();
            break;
        case 4: // Fractals
            if (anim == 0) activeAnim = new Mandelbrot();
            else if (anim == 1) activeAnim = new JuliaSet();
                        else if (anim == 2) activeAnim = new BurningShip();
            else if (anim == 3) activeAnim = new Sierpinski();
            else if (anim == 4) activeAnim = new KochSnowflake();
            else if (anim == 5) activeAnim = new BarnsleyFern();
            else if (anim == 6) activeAnim = new DragonCurve();
            else if (anim == 7) activeAnim = new HilbertCurve();
            else if (anim == 8) activeAnim = new LSystem();
            else if (anim == 9) activeAnim = new MZoom();
else activeAnim = new PlaceholderAnim();
            break;
        case 5: // Chaos Theory
            if (anim == 0) activeAnim = new LorenzAttractor();
            else if (anim == 2) activeAnim = new Bifurcation();
            else if (anim == 3) activeAnim = new DoublePendulum();
                        else if (anim == 1) activeAnim = new Rossler();
            else if (anim == 4) activeAnim = new LogisticMap();
            else if (anim == 5) activeAnim = new HenonMap();
            else if (anim == 6) activeAnim = new ArnoldCat();
            else if (anim == 7) activeAnim = new Lyapunov();
else activeAnim = new PlaceholderAnim();
            break;
        case 6: // N-Body
            if (anim == 1) activeAnim = new Boids();
                        else if (anim == 0) activeAnim = new ThreeBody();
            else if (anim == 2) activeAnim = new ParticleGas();
            else if (anim == 3) activeAnim = new GalaxyMerge();
            else if (anim == 4) activeAnim = new Swarm();
            else if (anim == 5) activeAnim = new SpringMass();
            else if (anim == 6) activeAnim = new Vortex();
            else if (anim == 7) activeAnim = new Charged();
else activeAnim = new PlaceholderAnim();
            break;
        case 7: // Fluid
            if (anim == 4) activeAnim = new FluidWave();
                        else if (anim == 0) activeAnim = new Smoke();
            else if (anim == 1) activeAnim = new VortexStreet();
            else if (anim == 2) activeAnim = new KHInstability();
            else if (anim == 3) activeAnim = new Convection();
            else if (anim == 5) activeAnim = new Turbulence();
            else if (anim == 6) activeAnim = new Soliton();
else activeAnim = new PlaceholderAnim();
            break;
        case 8: // Space
            if (anim == 0) activeAnim = new BlackHole();
            else if (anim == 1) activeAnim = new GalaxySpiral();
            else if (anim == 2) activeAnim = new Pulsar();
                        else if (anim == 3) activeAnim = new Eclipse();
            else if (anim == 4) activeAnim = new Comet();
            else if (anim == 5) activeAnim = new Supernova();
            else if (anim == 6) activeAnim = new GWaves();
            else if (anim == 7) activeAnim = new BigBang();
            else if (anim == 8) activeAnim = new Asteroids();
            else if (anim == 9) activeAnim = new Nebula();
else activeAnim = new PlaceholderAnim();
            break;
        case 9: // Math/Geo
            if (anim == 0) activeAnim = new Lissajous();
            else if (anim == 1) activeAnim = new Spirograph();
            else if (anim == 3) activeAnim = new VectorField();
                        else if (anim == 2) activeAnim = new Fourier();
            else if (anim == 4) activeAnim = new Voronoi();
            else if (anim == 5) activeAnim = new Contour();
            else if (anim == 6) activeAnim = new Quaternion();
            else if (anim == 7) activeAnim = new NonEuclidean();
            else if (anim == 8) activeAnim = new Sieve();
            else if (anim == 9) activeAnim = new Penrose();
else activeAnim = new PlaceholderAnim();
            break;
        default:
            activeAnim = new PlaceholderAnim();
            break;
    }

    if (activeAnim) {
        renderer.clear(' ', TFT_WHITE, TFT_BLACK);
        renderer.flush();
        activeAnim->init(renderer);
    }
}

void setup() {
    pinMode(14, OUTPUT);
    digitalWrite(14, HIGH);
    delay(100);

    auto cfg = M5.config();
    cfg.internal_mic = false; // Disable MIC (G0 conflicts with SD SCK)
    cfg.internal_spk = false; // Disable SPK
    M5.begin(cfg);
    
    M5.Display.setRotation(3); 
    M5.Display.setBrightness(128);

    if (!renderer.begin()) {
        while (1) delay(1);
    }

    menu.init();
}

void loop() {
    if (timer.ready()) {
        M5.update(); // Read inputs exactly once per 30fps frame

        // IMU Integration (Module 14)
        float ax = 0, ay = 0, az = 0;
        if (M5.Imu.isEnabled()) {
            M5.Imu.getAccel(&ax, &ay, &az);
        }
        
        float dt = timer.getDeltaTimeMs();
        
        if (!menu.isAnimationSelected()) {
            menu.update(renderer);
            if (menu.isAnimationSelected()) {
                startAnimation(menu.getSelectedCategory(), menu.getSelectedAnimation());
            }
        } else {
            if (activeAnim) {
                // Pass IMU data indirectly if needed, or activeAnim could read M5.Imu directly
                activeAnim->update(renderer, dt);
            }
            
            // Exit on any button press
            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnPWR.wasPressed() || M5.BtnC.wasPressed()) {
                menu.exitAnimation();
                if (activeAnim) {
                    delete activeAnim;
                    activeAnim = nullptr;
                }
            }
        }
        
        renderer.flush();
    }
}
