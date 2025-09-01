import pylsl

streams = pylsl.resolve_streams()
inlet = pylsl.StreamInlet(streams[0])

while True:
    sample, timestamp = inlet.pull_sample()
    print(timestamp, sample)
