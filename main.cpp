/*
 * main.cpp
 *
 *  Created on: Jun 19, 2016
 *
 *  Copyright (c) 2016 Simon Gustafsson (www.optisimon.com)
 *  Do whatever you like with this code, but please refer to me as the original author.
 */

#include "StreamProcessors/CappedStorageWaveform.hpp"
#include "StreamProcessors/MinMaxCheck.hpp"
#include "StreamProcessors/SlidingAverager.hpp"
#include "SDLWindow.hpp"
#include "SDLEventHandler.hpp"
#include "Stopwatch.hpp"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <getopt.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <atomic>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

int useMicDirectly_flag = 0;
int noGUI = 0;
int rpmDivisor = 1;
int requiredAmplitude = 3;
int verboseFlag = 0;
const char* inputAlsaDevice = "hw:0,0";
gchar* inputAudioFilename = NULL;

/* these are the caps we are going to pass through the appsink and appsrc */
const gchar *audio_caps =
		"audio/x-raw,format=S16LE,channels=2,rate=44100, layout=interleaved";

struct Stats {
	Stats() :
		rpm(0),
		filteredRpm(0),
		signalMax(0),
		signalMin(0),
		threshold(0),
		thresholdInPercentage(0),
		hysteresis(0)
	{ }
	float rpm;
	float filteredRpm;
	int signalMax;
	int signalMin;
	int threshold;
	int thresholdInPercentage;
	int hysteresis;
};

static std::atomic<int> gs_threshold_percentage(50);
static std::atomic<double> gs_rpm(0);
static std::atomic<bool> quit(false);


static std::vector<int16_t> g_period_waveform;
static bool g_period_waveform_updated = false;
Stats stats;
static std::mutex g_period_waveform_mutex;


void printTimeInformation()
{
	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	if (tmp == NULL)
	{
		perror("localtime");
		exit(1);
	}
	char outstr[200];
	if (strftime(outstr, sizeof(outstr), "%Y-%m-%d %H:%M:%S", tmp) == 0)
	{
		fprintf(stderr, "strftime returned 0");
		exit(1);
	}

	std::cout
	<< "date=" << outstr << "\n"
	<< "epoch=" << t << "\n";
}


// TODO: Consider updating waveform even when not triggering on anything. (roll mode)
class RPMCalculatorFromAudio {
public:
	RPMCalculatorFromAudio(int audioSampleRate, int divisor) :
		_audioSampleRate(audioSampleRate),
		_divisor(divisor),
		_periodCounter(0),
		_minMax(audioSampleRate / 5, 13),
		_slidingAverageRpmCalculator(10),
		_thresholdInPercentage(0),
		_threshold(0),
		_hysteresis(1),
		_amplitudeIsHighEnough(false),
		_isFirstTimestamp(true),
		_state(Uninitialized),
		_numStoredWaveforms(0),
		_numWaveformsBeforeDelivery(3)
{

}

	void check(int16_t sample)
	{
		_minMax.check(sample);
		_waveform.push(sample);

		_periodCounter++;
		switch(_state)
		{
		case Uninitialized:
			if (sample >= _threshold)
			{
				_state = WasAbove;
				_threshold = sample;
				_periodCounter = 0;
			}
			break;

		case WasBelow:
			if ((sample >= _threshold + _hysteresis) && _amplitudeIsHighEnough)
			{
				_state = WasAbove;
				_periodCounter = std::max<long>(_periodCounter, 1);

				double rpm = ((60.0 * _audioSampleRate) / _periodCounter ) / _divisor;

				gs_rpm = rpm;
				stats.rpm = rpm;
				_slidingAverageRpmCalculator.push(rpm);
				double filteredRpm = _slidingAverageRpmCalculator.getAverage();

				//
				// Time stamp the data
				//
				uint64_t nsecs = 0;
				uint64_t secs = 0;

				if (_isFirstTimestamp)
				{
					_isFirstTimestamp = 0;
					_stopwatch.restart();
					printTimeInformation();
				}
				else
				{
					_stopwatch.getElapsed(&secs, &nsecs);
				}


				if (verboseFlag)
				{
					std::cout
					<< "ts=" << secs << "." << nsecs/1000000
					<< ", PeriodCounter=" << _periodCounter
					<< ", rpm=" << rpm
					<< ", rpm_filtered=" << filteredRpm
					<< ", threshold=" << int(_threshold)
					<< ", hysteresis=" << int(_hysteresis)
					<< ", minMax.min=" << _minMax.getMin()
					<< ", minMax.max=" << _minMax.getMax()
					<< "\n";
				}
				else
				{
					std::cout
					<< "ts=" << secs << "." << nsecs/1000000
					<< ", rpm=" << rpm
					<< ", rpm_filtered=" << filteredRpm
					<< "\n";
				}

				_periodCounter = 0;
				_numStoredWaveforms++;

				if (_numStoredWaveforms >= _numWaveformsBeforeDelivery)
				{
					std::lock_guard<std::mutex> guard(g_period_waveform_mutex);

					const std::vector<int16_t>& waveform = _waveform.getWaveform();
					g_period_waveform.resize(waveform.size());

					int min = _minMax.getMin();
					int max = _minMax.getMax();

					for (size_t i = 0; i < waveform.size(); i++)
					{
						g_period_waveform[i] = waveform[i];
					}
					stats.signalMax = max;
					stats.signalMin = min;
					stats.threshold = _threshold;
					stats.thresholdInPercentage = _thresholdInPercentage;
					stats.hysteresis = _hysteresis;
					stats.filteredRpm = _slidingAverageRpmCalculator.getAverage();

					g_period_waveform_updated = true;
					_waveform.clear();
					_numStoredWaveforms = 0;
				}
			}
			break;

		case WasAbove:
			if (sample < _threshold - _hysteresis)
			{
				_state = WasBelow;
			}
			break;
		}

		_thresholdInPercentage = gs_threshold_percentage;
		float weight = _thresholdInPercentage * 0.01;
		_threshold = _minMax.getMax() * weight + _minMax.getMin() * (1 - weight);
		_hysteresis = (_minMax.getMax() - _minMax.getMin()) / 8;
		int amplitude = _minMax.getMax() - _minMax.getMin();
		_amplitudeIsHighEnough = amplitude > requiredAmplitude;
	}

private:
	int _audioSampleRate;
	int _divisor;
	long _periodCounter;

	MinMaxCheck _minMax;
	SlidingAverager _slidingAverageRpmCalculator;
	int _thresholdInPercentage;
	double _threshold;
	double _hysteresis;
	bool _amplitudeIsHighEnough;

	bool _isFirstTimestamp;
	Stopwatch _stopwatch;

	enum State {
		Uninitialized,
		WasBelow,
		WasAbove
	};

	State _state;

	int _numStoredWaveforms;
	const int _numWaveformsBeforeDelivery;

	CappedStorageWaveform _waveform;
};


typedef struct
{
	GMainLoop *loop;
	GstElement *source;
	RPMCalculatorFromAudio *check;
} ProgramData;

/* called when the appsink notifies us that there is a new buffer ready for
 * processing */
static GstFlowReturn
on_new_sample_from_sink (GstElement * elt, ProgramData * data)
{
	GstSample *sample;
	GstBuffer *buffer;

	// get the sample from appsink
	sample = gst_app_sink_pull_sample (GST_APP_SINK (elt));
	buffer = gst_sample_get_buffer (sample);

	// Map original buffer
	GstMapInfo info;
	gboolean isMapped = gst_buffer_map (buffer, &info, GST_MAP_READ);
	if (isMapped)
	{
		// Get the interesting bytes:
		int16_t buff[info.size/4];
		for (unsigned i = 0; i < info.size / 4; i++)
		{
			const int channel = 0;
			buff[i] = ((int16_t*)(info.data))[i*2 + channel];

			data->check->check(buff[i]);
		}

		gst_buffer_unmap(buffer, &info);
	}

	// we don't need the appsink sample anymore
	gst_sample_unref (sample);

	if (quit)
	{
		return GST_FLOW_EOS;
	}

	return GST_FLOW_OK;
}

/* called when we get a GstMessage from the source pipeline when we get EOS, we
 * notify the appsrc of it. */
static gboolean
on_source_message (GstBus * bus, GstMessage * message, ProgramData * data)
{
	switch (GST_MESSAGE_TYPE (message)) {
	case GST_MESSAGE_EOS:
		g_print ("# The source got dry\n");
		g_main_loop_quit (data->loop);
		break;
	case GST_MESSAGE_ERROR:
		g_print ("Received error\n");
		g_main_loop_quit (data->loop);
		break;
	default:
		break;
	}
	return TRUE;
}


void sdlDisplayThread()
{
	if (noGUI)
	{
		return;
	}

	SDLWindow win;
	SDLEventHandler eventHandler;
	eventHandler.setThresholdPercentage(gs_threshold_percentage);

	while(!quit)
	{
		win.drawTopText();

		{
			std::vector<int16_t> period_waveform;
			Stats statsCopy;
			{
				std::lock_guard<std::mutex> guard(g_period_waveform_mutex);
				period_waveform.resize(g_period_waveform.size());
				for (size_t i = 0; i < g_period_waveform.size(); i++)
				{
					period_waveform[i] = g_period_waveform[i];
				}

				statsCopy = stats;
			}

			float rpm = eventHandler.shouldDisplayFilteredRPM() ? stats.filteredRpm : stats.rpm;
			win.drawDigits(rpm, 4, /* showZeros */ false, /*showUnlitSegments*/ true,
					eventHandler.getDigitScaling());

			win.drawAdditionalStats(
					statsCopy.rpm,
					statsCopy.filteredRpm,
					statsCopy.signalMax,
					statsCopy.signalMin,
					statsCopy.threshold,
					statsCopy.hysteresis);

			int width = win.getWidth();
			int height = win.getHeight();
			double xScaling = width * 1.0 / period_waveform.size();

			const auto & convertY = [&](int sample) {
				int tmp = (sample - statsCopy.signalMin) * 255.0 / (statsCopy.signalMax - statsCopy.signalMin);
				return height - 1 - tmp;
			};

			for (int i = 0; i < int(period_waveform.size())-1; i++)
			{
				win.drawLine(
						i * xScaling,
						convertY(period_waveform[i]),
						(i+1) * xScaling,
						convertY(period_waveform[i+1]),
						255, 255, 255, 255
				);
			}

			// Draw the thresholds used during signal processing
			// TODO: draw a nice symbol for the hysteresis region as well?
			win.drawLine(
					0,
					convertY(statsCopy.threshold + statsCopy.hysteresis),
					width-1,
					convertY(statsCopy.threshold + statsCopy.hysteresis),
					0, 0, 255, 255);
			win.drawLine(
					0,
					convertY(statsCopy.threshold - statsCopy.hysteresis),
					width-1,
					convertY(statsCopy.threshold - statsCopy.hysteresis),
					0, 0, 255, 255);

		}

		win.flip();
		win.clear();
		usleep(20000);

		eventHandler.refresh();

		gs_threshold_percentage = eventHandler.getThresholdPercentage();
		if (eventHandler.shouldQuit())
		{
			quit = true;
		}


		bool wantFullscreen = eventHandler.shouldGoFullscreen();
		bool isFullscreen = win.isFullscreen();
		if (wantFullscreen != isFullscreen)
		{
			win.setFullscreenMode(wantFullscreen);

			for (int i = 8; i > 0 ; i--)
			{
				if (win.getHeight() > win.getDigitHeightInterdistance(i))
				{
					eventHandler.setDigitScaling(i);
					break;
				}
			}
		}
	}
}


int main (int argc, char *argv[])
{
	int showHelp_flag = 0;

	ProgramData *data = NULL;
	gchar *string = NULL;
	GstBus *bus = NULL;
	GstElement *testsink = NULL;

	while(true)
	{
		int c;
		static struct option long_options[] =
		{
				/* These options set a flag. */
				{"verbose", no_argument,   &verboseFlag, 1},
				{"blind",   no_argument,   &noGUI, 1},
				{"mic",     no_argument,   &useMicDirectly_flag, 1},
				{"device",  required_argument, 0, 'D'},
				{"amplitude", required_argument, 0, 'a'},
				{"rpm_divisor", required_argument, &rpmDivisor, 'd'},
				{"help",    no_argument,   &showHelp_flag, 1},
				/* These options don’t set a flag. We distinguish them by their indices. */
				{"file",    required_argument, 0, 'f'},
				{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long (argc, argv, "a:vbmd:D:hf:",
				long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			printf ("# *option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'v':
			verboseFlag = 1;
			break;

		case 'a':
			requiredAmplitude = std::stoi(optarg);
			printf("# Hysteresis forced to be at least %d\n", requiredAmplitude);
			break;

		case 'b':
			noGUI = 1;
			break;

		case 'D':
			inputAlsaDevice = optarg;
			printf("# Alsa device set to \"%s\"\n", inputAlsaDevice);
			break;

		case 'm':
			useMicDirectly_flag = 1;
			break;

		case 'd':
			rpmDivisor = std::stoi(optarg);
			printf("# RPM divisor set to %d\n", rpmDivisor);
			break;

		case 'h':
			showHelp_flag = 1;
			break;

		case 'f':
			inputAudioFilename = g_strdup (optarg);
			break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			abort ();
		}
	}

	if (showHelp_flag)
	{
		printf(
				"%s [options]\n"
				"-v, --verbose \n"
				"-b, --blind        Do not use the gui (text only output)\n"
				"-m, --mic          Use mic directly\n"
				"-D, --device       Specify alsa device (default hw:0,0)\n"
				"-d, --rpm_divisor  Number to divide pulse frequency with\n"
				"-a, --amplitude Number  minimum input waveform amplitude required before counting revolutions\n"
				"-h, --help\n"
				"-f, --file FILENAME.WAV (Analyzing a pre-recorded file)\n"
				"\n", argv[0]
		);
		return 1;
	}

	gst_init (&argc, &argv);

	data = g_new0 (ProgramData, 1);
	data->check = new RPMCalculatorFromAudio(44100, rpmDivisor);
	data->loop = g_main_loop_new (NULL, FALSE);

	std::thread thread1(sdlDisplayThread);

	/* setting up source pipeline, we read from a file and convert to our desired
	 * caps. */
	if (useMicDirectly_flag)
	{
		string = g_strdup_printf
				("alsasrc device=\"%s\" ! audioconvert ! audioresample ! appsink caps=\"%s\" name=testsink",
						inputAlsaDevice,
						audio_caps);
	}
	else
	{
		string = g_strdup_printf
				("filesrc location=\"%s\" ! wavparse ! audioconvert ! audioresample ! appsink caps=\"%s\" name=testsink",
						inputAudioFilename, audio_caps);
	}
	g_print("# Pipeline=\"%s\"\n", string);
	g_free (inputAudioFilename);
	data->source = gst_parse_launch (string, NULL);
	g_free (string);

	if (data->source == NULL) {
		g_print ("Bad source\n");
		return -1;
	}

	/* to be notified of messages from this pipeline, mostly EOS */
	bus = gst_element_get_bus (data->source);
	gst_bus_add_watch (bus, (GstBusFunc) on_source_message, data);
	gst_object_unref (bus);

	/* we use appsink in push mode, it sends us a signal when data is available
	 * and we pull out the data in the signal callback. If we want the appsink to
	 * push as fast as it can, we use sync=false */
	testsink = gst_bin_get_by_name (GST_BIN (data->source), "testsink");
	//g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", FALSE, NULL);
	g_object_set (G_OBJECT (testsink), "emit-signals", TRUE, "sync", TRUE, NULL);
	g_signal_connect (testsink, "new-sample",
			G_CALLBACK (on_new_sample_from_sink), data);
	gst_object_unref (testsink);

	/* launching things */
	gst_element_set_state (data->source, GST_STATE_PLAYING);

	/* let's run !, this loop will quit when the sink pipeline goes EOS or when an
	 * error occurs in the source or sink pipelines. */
	g_print ("# Let's run!\n");
	g_main_loop_run (data->loop);
	g_print ("# Going out\n");

	gst_element_set_state (data->source, GST_STATE_NULL);

	gst_object_unref (data->source);
	g_main_loop_unref (data->loop);
	g_free (data);


	quit = true;
	thread1.join();

	return 0;
}
