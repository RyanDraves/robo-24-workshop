import json
import logging
from typing import cast, Self, TypedDict
import queue
import threading
import time

from IPython.terminal import embed
from IPython.terminal import ipapp
import serial
from serial.tools import list_ports
from serial.tools import list_ports_common

DESCRIPTION = """Robo 24 Workshop Shell

This shell is used to interact with the ESP32-S3 board and the distance sensor.
Is consists of two clients that can be used to interact with the board:
- `client`: Regular client that requests a single measurement
  - `client.request_measurement()`: Request a single measurement
- `bad_client`: Bad client that floods the board with requests
  - `bad_client.request_measurement()`: Request a "single" measurement
"""


class Measurement(TypedDict):
    distance_mm: int
    timestamp_ms: int

class MesaurementRequest(TypedDict):
    pretty_please: int

class IPythonHandler(logging.Handler):
    def emit(self, record):
        print(self.format(record).strip())

class Esp32Serial:
    ESP32S3_VENDOR_PRODUCT_ID = '303a:1001'

    BAUD_RATE = 460800

    def __init__(self, port: str | None = None):
        # TODO: Make this automatically handle disconnects/reconnects
        self._serial = serial.Serial(None, self.BAUD_RATE, timeout=1)
        self._stop_byte = b'\r\n'
        self._port = port

        self._started = False
        self._read_thread = threading.Thread(target=self._read_loop, daemon=True)
        self._msg_queue = queue.Queue()

    @property
    def port(self) -> str:
        if self._port is None:
            self._port = self._find_esp32s3()
        return self._port

    def _find_esp32s3(self) -> str:
        devices: list[list_ports_common.ListPortInfo] = cast(
            list[list_ports_common.ListPortInfo],
            list(list_ports.grep(self.ESP32S3_VENDOR_PRODUCT_ID)),
        )
        if not devices:
            raise RuntimeError('ESP32-S3 not found')
        elif len(devices) > 1:
            device = devices[0]
        device = devices[0]

        logging.debug(f'ESP32-S3 found at {device.device}')
        return device.device

    def start(self) -> None:
        if self._started:
            return
        # Deferred port assignment to allow for context manager usage
        self._serial.port = self.port
        if not self._serial.is_open:
            self._serial.open()
        self._started = True
        self._read_thread.start()

    def stop(self) -> None:
        if not self._started:
            return
        self._serial.close()
        self._started = False

    def send(self, data: bytes) -> None:
        logging.debug('Tx: ' + ' '.join(f'{byte:02x}' for byte in data))
        self._serial.write(data + b'\r')

    def receive(self) -> bytes:
        return self._msg_queue.get()

    def _read_loop(self) -> None:
        while self._started:
            try:
                msg = self._receive()
                idx = msg.find(b'{')
                log_msg = msg[:idx]
                resp = msg[idx:] if idx != -1 else None
                if log_msg:
                    logging.info(log_msg.decode())
                if resp:
                    self._msg_queue.put(resp)
            except serial.SerialException:
                logging.error('Serial error occurred')
                break
            except:
                # Catch-all ignore the error (usually happens on an ungraceful exit)
                continue

    def _receive(self) -> bytes:
        buffer = bytes()
        while True:
            buffer += self._serial.read_until(self._stop_byte, size=255)
            if buffer.endswith(self._stop_byte):
                logging.debug('Rx: ' + ' '.join(f'{byte:02x}' for byte in buffer))
                return buffer


class Client:
    def __init__(self, node: Esp32Serial):
        self._serial = node

    def __enter__(self) -> Self:
        self._serial.start()
        return self

    def __exit__(self, exc_type, exc_value, traceback) -> None:
        self._serial.stop()

    def _send(self, data: bytes) -> None:
        self._serial.send(data)

    def _receive(self) -> bytes:
        return self._serial.receive()
    
    def request_measurement(self) -> Measurement:
        """Request a single measurement"""
        request: MesaurementRequest = {'pretty_please': 1}
        self._send(json.dumps(request).encode())
        return self._get_measurement()
    
    def _get_measurement(self) -> Measurement:
        """Receive a single measurement from the serial node"""
        start = time.time()
        while time.time() - start < 1:
            msg = self._receive().strip()
            try:
                measurement = json.loads(msg)
                return cast(Measurement, measurement)
            except json.JSONDecodeError:
                logging.warning('Invalid JSON received: %s', msg)
                continue
        raise RuntimeError('Failed to receive valid measurement')
    
    
class BadClient(Client):
    def request_measurement(self) -> Measurement:
        """Request 20 measurements in a tight loop and average them
        
        This ignores any restrictions on how fast we can poll the sensor
        """
        measurements: list[Measurement] = []
        start = time.time()
        for _ in range(20):
            measurements.append(super().request_measurement())
        end = time.time()

        logging.info(f'Received 20 measurements at {20 / (end - start):.2f} Hz')

        # Average the measurements and return the first timestamp
        avg_distance = sum(m['distance_mm'] for m in measurements) // len(measurements)
        m = measurements[0]
        m['distance_mm'] = avg_distance
        return m
        
    
if __name__ == '__main__':
    # Logging setup to print directly to console, instead of
    # mangling the input prompt
    logger = logging.getLogger()
    logger.setLevel(logging.INFO)
    handler = IPythonHandler()
    formatter = logging.Formatter('%(asctime)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    logger.addHandler(handler)

    node = Esp32Serial()

    # Start a shell with `client` and `bad_client` available
    client = Client(node)
    bad_client = BadClient(node)
    with client, bad_client:
        config = ipapp.load_default_config()
        config.InteractiveShellEmbed.colors = 'Linux'

        embed.embed(header=DESCRIPTION, config=config)
