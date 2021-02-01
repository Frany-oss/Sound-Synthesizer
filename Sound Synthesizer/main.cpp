#include <iostream>
#include "olcNoiseMaker.h"
#include "Envelope.h"
using namespace std;


atomic<double> dFrequencyOutput = 0.0;
double dOctaveBaseFrequency = 110.0; // Frequency of octave 
double d12hRootof2 = pow(2.0, 1.0 / 12.0); // Conversion to have all the 12 keys 
sEnvelopeADSR envelope;

#define SINE 0
#define SQUARE 1
#define TRIANGLE 2
#define SAW_ANA 3
#define SAW_DIG 4
#define NOISE 5

// Converts frequency (Hz) to angular velocity
double w(double dHz) {
	return dHz * 2.0 * PI;
}

double osc(double dHertz, double dTime, int nType = SINE, double dLFOAmplitude = 0.0, double dLFOHertz = 0.0) {

	double dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * (sin(w(dLFOHertz) * dTime));

	switch (nType) {
	case SINE: // Sine wave bewteen -1 and +1
		return sin(w(dHertz) * dTime);

	case SQUARE: // Square wave between -1 and +1
		return sin(w(dHertz) * dTime) > 0 ? 1.0 : -1.0;

	case TRIANGLE: // Triangle wave between -1 and +1
		return asin(sin(w(dHertz) * dTime)) * (2.0 / PI);

	case SAW_ANA: { // Saw wave (analogue / warm / slow)
		double dOutput = 0.0;

		for (double n = 1.0; n < 10.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;

		return dOutput * (2.0 / PI);
	}

	case SAW_DIG: // Saw Wave (optimised / harsh / fast)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));


	case NOISE: // Pseudorandom noise
		return 2.0 * ((double)rand() / (double)RAND_MAX) - 1.0;

	default:
		return 0.0;
	}
}

struct instrument {
	double dVolume;
	sEnvelopeADSR env;

	virtual double sound(double dTime, double dFrequency) = 0;

};

struct bell : public instrument {
	bell() {
		env.dAttackTime = 0.0;
		env.dDecayTime = 1.0;
		env.dReleaseTime = 1.0;
		env.dSustainAmplitude = 0.0;
		env.dStartAmplitude = 1.0;
	}

	double sound(double dTime, double dFrequency) {
		double dOutput = env.getAmplitude(dTime) *
			(
				+1.0 * osc(dFrequencyOutput * 2, dTime, SINE, 5.0, 0.001)
				+ 0.5 * osc(dFrequencyOutput * 2.0, dTime, SINE)
				+ 0.25 * osc(dFrequencyOutput * 3.0, dTime, SINE)
				);
		return dOutput;
	}
};

struct harmonica : public instrument {
	harmonica() {
		env.dAttackTime = 0.05;
		env.dDecayTime = 0.01;
		env.dReleaseTime = 0.20;
		env.dSustainAmplitude = 0.8;
		env.dStartAmplitude = 1.0;
	}

	double sound(double dTime, double dFrequency) {
		double dOutput = env.getAmplitude(dTime) *
			(
				+ 1.0 * osc(dFrequencyOutput, dTime, SQUARE, 5.0, 0.001)
				+ 0.5 * osc(dFrequencyOutput * 1.5, dTime, SQUARE)
				+ 0.25 * osc(dFrequencyOutput * 2.0, dTime, SQUARE)
				+ 0.05 * osc(0, dTime, NOISE)
				);
		return dOutput;
	}
};

instrument* voice = nullptr;

// Fucntion that makes the sound (in Hrz -> note A = 440 Hrz)
double MakeNoise(double dTime) {

	//1.0 * osc(dFrequencyOutput * 0.5, dTime, SINE, 5.0, 0.1) // BASS
	

	double dOutput = voice->sound(dTime, dFrequencyOutput);
	return dOutput * 0.4; // 0.6 -> Master Volume
}


int main() {

	wcout << "------ Synthesizer ------" << endl;

	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto i : devices) wcout << "Found Output Device: " << i << endl;
	wcout << "Using Device: " << devices[0] << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// voice = new bell();
	voice = new harmonica();

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	// Display a keyboard with the layout
	wcout << endl <<
		" ------------------ PLAY WITH THE PIANO!!! -----------------"  << endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |  " << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |  " << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"| (A) | (B) | (C) | (D) | (E) | (F) | (G) | (A) | (B) | (C) |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	bool bKeyPressed = false;
	int nCurrentKey = -1;

	while (1) {
		bool bKeyPressed = false;
		for (int i = 0; i < 16; ++i) {
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[i])) & 0x8000) {
				if (nCurrentKey != i) {
					// 0->A,  1->A#, 2->B, ...., 12->A2
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12hRootof2, i);
					voice->env.NoteOn(sound.GetTime());
					cout << "\rNote On : " << sound.GetTime() << "s " << dFrequencyOutput << "Hz";
					nCurrentKey = i;
				}
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed) {
			if (nCurrentKey != -1) {
				cout << "\rNote Off: " << sound.GetTime() << "s                        ";
				voice->env.NoteOff(sound.GetTime());
				nCurrentKey = -1;
			}
		}
	}

	return 0;
}
