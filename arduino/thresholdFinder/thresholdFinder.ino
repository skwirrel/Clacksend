// This is the Serial object to be used for debug
#define SERIAL_PORT Serial
// Use one or other of these debug modes - not both!

// To get a graph of all the samples use this one...
//   sudo stdbuf -o0 cat /dev/ttyACM0 | head -1500 > /tmp/gnuplotData.csv; gnuplot -p debugSamples.gnuplot
#define DEBUG_SAMPLES false

// To get a frequency distribution histogram use this one...
//   sudo stdbuf -o0 cat /dev/ttyACM0 | php debugPlotter.php 
#define DEBUG_THRESHOLDS true


// The sample average count is stored as a power of 2 i.e. 2^5 = 32
#define SAMPLE_AVERAGE_COUNT_POWER 4
#define LDR_PIN A0
#define NUM_BUCKETS 100
#define ADC_MAX 1024

const byte SAMPLE_AVERAGE_COUNT = pow(2,SAMPLE_AVERAGE_COUNT_POWER);

void initialiseArray(short array[], size_t len, int value = 0) {
  int i;
  for (i = 0; i < len; i++) {
    array[i] = value;
  }
}

short getReading() {
  static short avgBuffer[SAMPLE_AVERAGE_COUNT];
  static byte bufferPos = 0;
  static bool bufferInitialised = 0;
  static int sum;
  short int ldrReading, avg;

  if (!bufferInitialised) {
    initialiseArray(avgBuffer, SAMPLE_AVERAGE_COUNT);
    bufferInitialised = true;
  }

  ldrReading = (short)analogRead(LDR_PIN);
  sum -= avgBuffer[bufferPos];
  sum += ldrReading;

  avgBuffer[bufferPos] = ldrReading;
  if (++bufferPos == SAMPLE_AVERAGE_COUNT) bufferPos = 0;

  avg = sum >> SAMPLE_AVERAGE_COUNT_POWER;
  short max = 0;
  short min = avgBuffer[0];  
  
  for( byte i=0; i<SAMPLE_AVERAGE_COUNT; i++) {
    if (avgBuffer[i]>max) max=avgBuffer[i];
    else if (avgBuffer[i]<min) min=avgBuffer[i];
  }
  short median = (max + min) >> 1;

  #if (DEBUG_SAMPLES)
    SERIAL_PORT.print(ldrReading);
    SERIAL_PORT.print(',');
    SERIAL_PORT.print(avg);
    SERIAL_PORT.print(',');
    SERIAL_PORT.print(max);
    SERIAL_PORT.print(',');
    SERIAL_PORT.print(min);
    SERIAL_PORT.print(',');
    SERIAL_PORT.print(median);
    SERIAL_PORT.print("\n");
  #endif
  
  return max;
}

short findPeak(short buckets[]) {
  // First find the peak
  short max = 0;
  short bucket = 0;
  int i;
  for (i = 0; i < NUM_BUCKETS; i++) {
    if (buckets[i] > max) {
      max = buckets[i];
      bucket = i;
    }
  }

  short threshold = max >> 1; // Devide max by two
  //threshold = 9999;
  
  // then remove it - start by removing the part of the peak to the right
  short lastValue = buckets[bucket];
  for (i = bucket + 1; i < NUM_BUCKETS; i++) {
    if (buckets[i] > threshold || buckets[i] < lastValue) {
      lastValue = buckets[i];
      buckets[i] = 0;
    } else {
      break;
    }
  }

  // then remove the part of the peak to the left
  lastValue = buckets[bucket];
  for (i = bucket; i >= 0; i--) {
    if (buckets[i] > threshold || buckets[i] < lastValue) {
      lastValue = buckets[i];
      buckets[i] = 0;
    } else {
      break;
    }
  }

  return bucket;
}

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  static short thresholds[5];
  static short buckets[NUM_BUCKETS];
  static short smoothedBuckets[NUM_BUCKETS];
  static bool initialised = false;
  static short count = 0;

  if (!initialised) {
    initialised = true;
    initialiseArray(buckets, NUM_BUCKETS);
  }

  // put your main code here, to run repeatedly:
  int reading = getReading();
  int bucket = (int)((float)reading / (float)ADC_MAX * (float)NUM_BUCKETS);
  //SERIAL_PORT.println(bucket);
  buckets[bucket]++;

  if (count == 5000) {
    short i;

    smoothedBuckets[0] = buckets[0];
    smoothedBuckets[1] = buckets[1];
    smoothedBuckets[NUM_BUCKETS - 1] = buckets[NUM_BUCKETS - 1];
    smoothedBuckets[NUM_BUCKETS - 2] = buckets[NUM_BUCKETS - 2];

    for (i = 2; i < NUM_BUCKETS - 2; i++) {
      smoothedBuckets[i] = (buckets[i - 2] + buckets[i - 1] + buckets[i] + buckets[i + 1] + buckets[i + 2]) / 5;
    }

    // find the peaks
    short lastPeak = 0;
    short peaks[5];
    for (i = 0; i < 5; i++) {
      peaks[i] = findPeak(smoothedBuckets);
    }

    #if (DEBUG_THRESHOLDS)
      // hishestPeak is just used in debug output later on
      short highestPeak = peaks[0];
    #endif
    
    // Sort the peaks - just uses a simple bubble sort 
    // 
    byte numSwaps;
    do {
      numSwaps = 0;
      for (i = 0; i < 5 - 1; i++) {
        if (peaks[i] > peaks[i + 1]) {
          numSwaps++;
          int tmp = peaks[i];
          peaks[i] = peaks[i + 1];
          peaks[i + 1] = tmp;
        }
      }
    } while (numSwaps > 0);

    for (i = 0; i < 4; i++) {
      thresholds[i] = (peaks[i] + peaks[i + 1]) >> 1;
    }

    #if (DEBUG_THRESHOLDS)
      short nextThreshold = 0;
      for (i = 0; i < NUM_BUCKETS; i++) {
        SERIAL_PORT.print(buckets[i]);
        SERIAL_PORT.print(",");
        SERIAL_PORT.print(smoothedBuckets[i]);
        SERIAL_PORT.print(",");
        if (i==thresholds[nextThreshold]) {
          // Put a "blip" on the graph to show the threshold level
          SERIAL_PORT.print(highestPeak);
          nextThreshold++;
        } else {
          SERIAL_PORT.print(0);
        }
        SERIAL_PORT.print("\n");
      }
      SERIAL_PORT.print("==================================\n");
    #endif

    initialiseArray(buckets, NUM_BUCKETS);
    count = 0;
  }

  count++;

  delay(random(1,4));
}