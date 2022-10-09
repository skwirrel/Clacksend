# What is Threshold Finder

Before we can extract data from the lightstream we need to be able distinguish one colour/intensity from another. My ideal outcome would be able to use a single light sensor so as not to require too many components and not tie up too many of the ADC pins on the microcontroller.

The code in this folder is what I am using to experiment with different ways of distinguishing different light intensities.

# Challenges
There are 2 main problems
1. The simple and obvious problem... the light level will change depending on how far the reader (light sensor) is from the writer (phone/laptop screen). The image below shows a graph of light level readings taken with the sensor write up against the screen...
![alt](testSignalFullBrigthness_closeUp.png)
... and this image shows how these light levels change when the sensor is further from the screen...
![alt](testSignalFullBrigthness_furtherAway.png)
You can still clearly identify the steps but they are at different values.
2. *The following is my current hypothesis*... The second and more challenging problem is dealing with screen brightness. Most screens these days are backlit. That means the brightness of the screen comes from white LED's behind the screen. When you turn down the brightness of your screen all that happens is the brightness of the backlight LED's is turned down. LED's can only be on or off, so reducing the brightness involves using pulse width modulation (PWM) i.e. turning the LED on and off very quickly. This seems to cause us a problem. The graphs above were sampled at full brightness. Here is a graph taken at low brightness with the reader right up against the screen.
![alt](testSignalLowBrigthness_closeUp.png)
... The purple line is the real signal, the blue line is a rolling average. The signal appears to be extremely noisy. What I think is happening here is that we are reading the light level so quickly that some of the time we are reading the light level when the backlight LED's are switched off. If you look at the peaks of the purple lines then we are seeing a sine wave. I suspect this is due to the sampling rate and the LED flash rate being slightly out of sync. Also... if you look at the lowest level there is very little noise here - that is because this is black i.e. none of the light from the backlight LED's is being let through - hence no PWM effect. If we move the reader further from the screen then the sample gets even more messy...
![alt](testSignalLowBrigthness_furtherAway.png)

The first problem I have pretty much solved and will come back to.

The second problem is tricky. The graphs above show a test sequence: 0,1,2,3,4 etc. Up to now I have been encoding the data as a sequence of different brightnesses e.g. 4,2,1,0,2,3... A real data sample at low brightness looks like this...
![alt](realSignalLowBrigthness_closeUp.png)
The clear steps are completely gone and distinguishing the levels is now very hard.

The graph above also include a rolling maximum line (pale blue) and a rolling minimum line (yellow).

Part of the problem here is the time it takes the light sensor to register the change - I think it takes it longer when the change is greater. So the shape of a 4 is different if the previous level was 3 compared to a 4 when the previous level was 0.

The behaviour of the sensor seems much more predictable if we pull it back down to zero light level between each sample...
![realSignalPulsedLowBrigthness_closeUp](realSignalPulsedLowBrigthness_closeUp.png)

Here we see a fairly consistent and easily distinguished shape to the data which is encoded as differing height peaks inbetween the zero periods. This is especially true if we look at the light blue line which shows the rolling maximum over 16 samples.

# Next Steps
I think the way forward is as follows:
1. Encode the data as pulses between zero's as shown above
2. Rather than looking for the step, look for the leading and trailing edge of the pulse.
3. When the leading edge of the pulse is detected then start recording samples
4. When the trailing edge of the pulse is detected then stop recording samples and look for the maximum sample encountered in that pulse (we can also record the pulse length at the same time).
5. We can now use both the level and the length of the pulse (using the zero period as a metric for pulse length).
This approach should allow us to encode 6 bits of data in a single pulse (1 bit in pulse duration, and 2 bits from pulse intensity).
BTW the pulse lenght in the graph above is 70ms.

# Threshold Detection

As mentioned above, the other challenge is identifying the step levels. The approach I am taking to this is to take a number of samples and draw a frequency distribution histogram. The steps should be identified as 5 distinct peaks in this histogram. Since we are now adding a zero between every other data point there should be 4 times as many zeros as any of the other frequency peaks.

The graph below shows the frequency distribution histogram for 1250 samples taken from a rolling maximum (over 8 samples).

![ReadingFrequencyDistributionLowBrightness](ReadingFrequencyDistributionLowBrightness.png)

The purple line shows the counts for each bucket and the blue lines show the computed threshold value.

The threshold finder algorithm looks for the highest bucket, records this as a peak and then zero's out this peak. Then goes round again looking for the next highest bucket. It does this 5 times zeroing out each peak as it goes. Once it has 5 peaks it finds the midpoints between these which are the threshold values.

The graph above shows that 4 thresholds have been identified, but the peaks are not very distinct due to the noisy signal.

I wanted to improve on this so I processed the input samples as shown in the graph below...
![findingMaximaAndMinima](findingMaximaAndMinima.png)
(IGNORE THE LINE LABLES ON THIS GRAPH)

This graph shows
- In purple: the rolling maximum (computed over just 8 - just enough to remove the noise from the PWM) in purple. This curve is shifted back in time a bit so that it lines up with the next curve (because they are averaged over different periods)...
- In blue: the rolling average of the rolling maximum (computed over 32 samples to give a nice smooth curve)
- In pale blue: The rate of change of the blue line
- In yellow: The points at which the pale blue line crosses the x axis - these are the maxima and minima

If we now draw up a frequency distribution histogram again, but this time only take the readings from where we have a maximum or a minimum, then we should get much better clustering of the readings around the step levels. Furthermore we can multiply the maximum counts by 4 to comensate for the fact that there are more minimums than maximums.

![ReadingFrequencyDistributionLowBrightness_improved](ReadingFrequencyDistributionLowBrightness_improved.png)

We can optimise this even more as follows... if we look at the sample graph (2 graphs up) every time we encounter a maximum (blue peak), we start looking for the **highest** value we encounter until the next minimum (blue dip). Every time we encounter a minimum (blu dip) we start looking for the **lowest** value until the next maximum (blue peak). Bearing in mind that the purple line has been shifted and should be half a cycle to the right. This approach means that we are taking the highest value from the purple peaks and the lowest value from the purple dips. If we now draw the frequency histogram for these values we get the following...

![ReadingFrequencyDistributionLowBrightness_improvedAgain](ReadingFrequencyDistributionLowBrightness_improvedAgain.png)

This shows clear peaks at the step values (purple line) and the thresholds calculated between these (blue values).

The same graph when measured with the screen at full brightness shows even more distinct peaks...

![ReadingFrequencyDistributionHighBrightness_improvedAgain](ReadingFrequencyDistributionHighBrightness_improvedAgain.png)

# Longer Pulses

It would be nice to squeeze some extra data into the signal by varying the pulse length. If we now introduce longer pulses lets see what we get....
![testSignalLowBrigthnessLongPulese_closeUp](testSignalLowBrigthnessLongPulese_closeUp.png)
Oh dear. The maximum and minimum detection has broken during the flat periods - here we get lots of little local maxima and minima (yellow spikes).

