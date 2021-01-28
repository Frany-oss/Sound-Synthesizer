#include <iostream>
#include "olcNoiseMaker.h"
#include "Envelope.h"
using namespace std;



atomic<double> dFrequencyOutput = 0.0;
double dOctaveBaseFrequency = 110.0; // Frequency of octave 
double d12hRootof2 = pow(2.0, 1.0 / 12.0); // Conversion to have all the 12 keys 
sEnvelopeADSR envelope;

// Converts frequency (Hz) to angular velocity
double w(double dHz) {
	return dHz * 2.0 * PI;
}

double osc(double dHertz, double dTime, int nType) {

	switch (nType) {
	case 0: // Sine wave
		return sin(w(dHertz) * dTime); 

	case 1: // Square wave
		return sin(w(dHertz) * dTime) > 0.0 ? 1.0 : -1.0; 

	case 2: // Triangle wave
		return asin(sin(w(dHertz) * dTime)) * 2.0 / PI;

	case 3: { // Saw wave (slower)
		double dOutput = 0.0;
		for (double n = 1.0; n < 10.0; n++)
			dOutput += (sin(n * w(dHertz) * dTime)) / n;
		return dOutput * (2.0 / PI);
	}
	case 4: // Saw wave (computational way -> faster)
		return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));
		
	case 5: // Pseudo Random Noise
		return 2.0 * ((double) rand() / (double) RAND_MAX) - 1.0;

	default:
		return 0;
	}

}

// Fucntion that makes the sound (in Hrz -> note A = 440 Hrz)
double MakeNoise(double dTime) {

	double dOutput = envelope.getAmplitude(dTime) * osc(dFrequencyOutput, dTime, 3);

	return dOutput * 0.4; // 0.6 -> Master Volume
}


int main() {

	wcout << "------ Synthesizer ------" << endl;

	// Get all sound hardware
	vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Display findings
	for (auto i : devices) wcout << "Found Output Device: " << i << endl;

	// Create sound machine
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Link noise function with sound machine
	sound.SetUserFunction(MakeNoise);

	// Display a keyboard with the layout
	wcout << endl <<
		" ------------------ PLAY WITH THE PIANO!!! -----------------"  << endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |  " << endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |  " << endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

	bool bKeyPressed = false;
	int nCurrentKey = -1;

	while (1) {
		for (int i = 0; i < 16; ++i) {
			if (GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe"[i])) & 0x8000) {
				if (nCurrentKey != i) {
					// 0->A,  1->A#, 2->B, ...., 12->A2
					dFrequencyOutput = dOctaveBaseFrequency * pow(d12hRootof2, i);
					envelope.NoteOn(sound.GetTime());
					cout << "\rNote On : " << sound.GetTime() << "s " << dFrequencyOutput << "Hz";
					nCurrentKey = i;
				}
				bKeyPressed = true;
			}
		}

		if (!bKeyPressed) {
			if (nCurrentKey != -1) {
				cout << "\rNote Off: " << sound.GetTime() << "s                        ";
				envelope.NoteOff(sound.GetTime());
				nCurrentKey = -1;
			}
		}
	}

	return 0;
}
