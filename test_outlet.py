import pylsl
import random
import time


def main():
    print(f"using liblsl version {pylsl.library_version()}")

    # create stream info
    name = "LslTestOutlet"
    num_channels = 8
    uid = "ddadfd62"
    stream_info = pylsl.StreamInfo(
        name=name,
        type="EEG",
        channel_count=num_channels,
        nominal_srate=500,
        channel_format=pylsl.cf_int8,
        source_id=uid,
    )

    # create xml metadata
    desc = stream_info.desc()
    desc.append_child_value("manufacturer", "LSL")
    for i in range(num_channels):
        channel_desc = desc.append_child("channel")
        channel_desc.append_child_value("label", f"CH{i + 1}")
        channel_desc.append_child_value("unit", "microvolts")
        channel_desc.append_child_value("type", "EEG")

    # create the outlet
    outlet = pylsl.StreamOutlet(
        info=stream_info,
        chunk_size=0, # no chunks, send each sample one-by-one
        max_buffered=360,
    )

    print("Waiting for consumers...")
    while not outlet.wait_for_consumers(timeout=120):
        pass

    print("Sending data...")
    t = 0
    while outlet.have_consumers():
        # samples = [random.random() * 3.0 - 1.5 for _ in range(num_channels)]
        samples = [i for i in range(num_channels)]
        outlet.push_sample(samples)

        time.sleep(1 / 500)
        t += 1

    print("No more consumers. Shutting down...")

if __name__ == "__main__":
    main()
