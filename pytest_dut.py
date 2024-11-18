from pytest_embedded import Dut

import shell
import time

def test_measurement(
    dut: Dut
) -> None:
    client = shell.BadClient(shell.Esp32Serial())
    with client:
        measurement = client.request_measurement()
        assert isinstance(measurement, dict)

def test_measurement_rate(
    dut: Dut
) -> None:
    client = shell.BadClient(shell.Esp32Serial())
    with client:
        start = time.time()
        _ = client.request_measurement()
        end = time.time()
        measurements_taken = 20
        tolerance = 0.010
        # The datasheet suggests a >60ms measurement cycle
        diff = end - start
        expected_duration = measurements_taken * 0.060
        assert diff > expected_duration - tolerance