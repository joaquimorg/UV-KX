#
# UV-Kx driver (c) 2025 joaquim.org
#
#
# Adapted For UV-K5 EGZUMER custom software By EGZUMER, JOC2
#
# based on template.py Copyright 2012 Dan Smith <dsmith@danplanet.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


from re import A
import struct
import logging

from chirp import chirp_common, directory, bitwise, memmap, errors, util
from chirp.settings import RadioSetting, RadioSettingGroup, \
    RadioSettingValueBoolean, RadioSettingValueList, \
    RadioSettingValueInteger, RadioSettingValueString, \
    RadioSettings, RadioSettingSubGroup

LOG = logging.getLogger(__name__)

# Show the obfuscated version of commands. Not needed normally, but
# might be useful for someone who is debugging a similar radio
DEBUG_SHOW_OBFUSCATED_COMMANDS = False

# Show the memory being written/received. Not needed normally, because
# this is the same information as in the packet hexdumps, but
# might be useful for someone debugging some obscure memory issue
DEBUG_SHOW_MEMORY_ACTIONS = True

## #######################################################################################

CHAN_MAX = 230  
MEM_SIZE = 0x2000
PROG_SIZE_V = 0x0050
PROG_SIZE_U = 0x0140
PROG_SIZE = 0x2000
PROG_SIZEM = 0x1e00
START_MEM = 0x0050

MEM_BLOCK = 0x80  # largest block of memory that we can reliably write

## #######################################################################################

CTCSS_TONES = [
    67.0, 69.3, 71.9, 74.4, 77.0, 79.7, 82.5, 85.4,
    88.5, 91.5, 94.8, 97.4, 100.0, 103.5, 107.2, 110.9,
    114.8, 118.8, 123.0, 127.3, 131.8, 136.5, 141.3, 146.2,
    151.4, 156.7, 159.8, 162.2, 165.5, 167.9, 171.3, 173.8,
    177.3, 179.9, 183.5, 186.2, 189.9, 192.8, 196.6, 199.5,
    203.5, 206.5, 210.7, 218.1, 225.7, 229.1, 233.6, 241.8,
    250.3, 254.1, 
]

# lifted from ft4.py
DTCS_CODES = [
    23,  25,  26,  31,  32,  36,  43,  47,  51,  53,  54,
    65,  71,  72,  73,  74,  114, 115, 116, 122, 125, 131,
    132, 134, 143, 145, 152, 155, 156, 162, 165, 172, 174,
    205, 212, 223, 225, 226, 243, 244, 245, 246, 251, 252,
    255, 261, 263, 265, 266, 271, 274, 306, 311, 315, 325,
    331, 332, 343, 346, 351, 356, 364, 365, 371, 411, 412,
    413, 423, 431, 432, 445, 446, 452, 454, 455, 462, 464,
    465, 466, 503, 506, 516, 523, 526, 532, 546, 565, 606,
    612, 624, 627, 631, 632, 654, 662, 664, 703, 712, 723,
    731, 732, 734, 743, 754
]

UVK5_POWER_LEVELS = [chirp_common.PowerLevel("Low",  watts=1.00),
                     chirp_common.PowerLevel("Med",  watts=2.50),
                     chirp_common.PowerLevel("High", watts=5.00)]

## #######################################################################################

""""
    EEPROM Layout Overview:
    This comment block describes how data is organized within the EEPROM.

    [0x0000 - 0x004F] : Global Radio Settings (defined by SETTINGS struct, approx 80 bytes)
    [0x0050 - 0x1D0F] : Memory Channels (e.g., 230 channels * 32 bytes/channel = 7360 bytes = 0x1CC0 bytes)
                       (End address would be 0x0050 + 0x1CC0 - 1 = 0x1D0F)
    [0x1D10 - 0x1DFF] : Empty / Unused space
    [0x1E00 - ... ]   : Calibration Data
    ...
    [0x1FFF]          : End of a typical 8KB EEPROM (like 24C64)
"""

MEM_FORMAT = """
#seekto 0x0000;
struct {
    ul16 version;
    u8 battery_type:2,
       busy_lockout:1,
       beep:1,
       backlight_level:4;
    u8 backlight_time:4,
       mic_gain_db:4;
    u8 lcd_contrast:4,
       tx_timeout:4;
    u8 battery_save:4,
       backlight_mode:2,
       vfo_selected:2;
    ul16 memory[2];
    struct {
        ul32 rx_frequency;
        u8   rx_code_type;
        u8   rx_code;
        ul32 tx_frequency;
        u8   tx_code_type;
        u8   tx_code;
        char name[10];
        ul16 channel_id;
        u8 squelch:4,
           step:4;
        u8 modulation:4,
           bandwidth:4;
        u8 power:2,
           shift:2,
           repeater_ste:1,
           ste:1,
           compander:2;
        u8 roger:4,
           pttid:4;
        u8 rxagc:6,
           vfo_reserved_bits:2;
        u8 reserved_bytes[3];
    } vfo[2];
    u8 show_vfo[2];
    u8 settings_reserved[4];
} settings;

#seekto 0x0050;
struct {
    ul32 rx_frequency;
    u8   rx_code_type;
    u8   rx_code;
    ul32 tx_frequency;
    u8   tx_code_type;
    u8   tx_code;
    char name[10];
    ul16 channel_id;
    u8 squelch:4,
       step:4;
    u8 modulation:4,
       bandwidth:4;
    u8 power:2,
       shift:2,
       repeater_ste:1,
       ste:1,
       compander:2;
    u8 roger:4,
       pttid:4;
    u8 rxagc:6,
       channel_reserved_bits:2;
    u8 reserved_bytes[3];
} channel[230];
"""

FREQ_UNITS = 10
MAX_FREQ_VALUE = 0xFFFFFFFF
STEP_TABLE_KHZ = [0.5, 1.0, 2.5, 5.0, 6.25, 10.0, 12.5, 15.0, 20.0, 25.0,
                  50.0, 100.0, 500.0]

MODE_MAP = {
    0: "FM",
    1: "AM",
    2: "LSB",
    3: "USB",
    4: "FM",   # BYP
    5: "FM",   # RAW
    6: "WFM",
    7: "FM",   # Preset
}

SHIFT_NONE = 0
SHIFT_PLUS = 1
SHIFT_MINUS = 2

CODETYPE_NONE = 0
CODETYPE_TONE = 1
CODETYPE_DTCS = 2
CODETYPE_INV_DTCS = 3

BATTERY_TYPES = ["1600 mAh", "2200 mAh", "3500 mAh"]
BACKLIGHT_TIME_OPTIONS = ["Off", "On", "5s", "10s", "15s",
                          "20s", "30s", "1m", "2m", "4m"]
MIC_DB_OPTIONS = ["+1.1 dB", "+4.0 dB", "+8.0 dB", "+12.0 dB", "+15.1 dB"]
LCD_CONTRAST_OPTIONS = ["100", "110", "120", "130", "140", "150",
                        "160", "170", "180", "190", "200"]
TX_TIMEOUT_OPTIONS = ["30s", "1m", "2m", "4m", "6m", "8m"]
BACKLIGHT_MODE_OPTIONS = ["Off", "TX", "RX", "TX/RX"]
BANDWIDTH_OPTIONS = ["26", "23", "20", "17", "14", "12", "10", "9", "7", "6"]
RXAGC_OPTIONS = ["-43", "-40", "-38", "-35", "-33", "-30", "-28", "-25", "-23",
                 "-20", "-18", "-15", "-13", "-11", "-9", "-6", "-4", "-2", "AUTO"]

## #######################################################################################

def min_max_def(value, min_val, max_val, default):
    """returns value if in bounds or default otherwise"""
    if min_val is not None and value < min_val:
        return default
    if max_val is not None and value > max_val:
        return default
    return value


# --------------------------------------------------------------------------------
# nibble to ascii
def hexasc(data):
    res = data 
    if res<=9:
        return chr(res+48)
    elif data == 0xA:
        return "A"
    elif data == 0xB:
        return "B"
    elif data == 0xC:
        return "C"
    elif data == 0xD:
        return "D"    
    elif data == 0xF:
        return "F"
    else:
        return " "

# --------------------------------------------------------------------------------
# nibble to ascii
def ascdec(data):

    if data == "0":
        return 0
    elif data == "1":
        return 1
    elif data == "2":
        return 2
    elif data == "3":
        return 3
    elif data == "4":
        return 4
    elif data == "5":
        return 5
    elif data == "6":
        return 6
    elif data == "7":
        return 7
    elif data == "8":
        return 8
    elif data == "9":
        return 9
    elif data == "A":
        return 10
    elif data == "B":
        return 11
    elif data == "C":
        return 12
    elif data == "D":
        return 13
    elif data == "F":
        return 15
    else:
        return 14


# --------------------------------------------------------------------------------
# the communication is obfuscated using this fine mechanism
def xorarr(data: bytes):
    tbl = [22, 108, 20, 230, 46, 145, 13, 64, 33, 53, 213, 64, 19, 3, 233, 128]
    x = b""
    r = 0
    for byte in data:
        x += bytes([byte ^ tbl[r]])
        r = (r+1) % len(tbl)
    return x

# --------------------------------------------------------------------------------
# if this crc was used for communication to AND from the radio, then it
# would be a measure to increase reliability.
# but it's only used towards the radio, so it's for further obfuscation
def calculate_crc16_xmodem(data: bytes):
    poly = 0x1021
    crc = 0x0
    for byte in data:
        crc = crc ^ (byte << 8)
        for i in range(8):
            crc = crc << 1
            if (crc & 0x10000):
                crc = (crc ^ poly) & 0xFFFF
    return crc & 0xFFFF

# --------------------------------------------------------------------------------
def _send_command(serport, data: bytes):
    """Send a command to UV-K5 radio"""
    LOG.debug("Sending command (unobfuscated) len=0x%4.4x:\n%s" %
              (len(data), util.hexprint(data)))

    crc = calculate_crc16_xmodem(data)
    data2 = data + struct.pack("<H", crc)

    command = struct.pack(">HBB", 0xabcd, len(data), 0) + \
        xorarr(data2) + \
        struct.pack(">H", 0xdcba)
    if DEBUG_SHOW_OBFUSCATED_COMMANDS:
        LOG.debug("Sending command (obfuscated):\n%s" % util.hexprint(command))
    try:
        result = serport.write(command)
    except Exception:
        raise errors.RadioError("Error writing data to radio")
    return result

# --------------------------------------------------------------------------------
def _receive_reply(serport):
    header = serport.read(4)
    if len(header) != 4:
        LOG.warning("Header short read: [%s] len=%i" %
                    (util.hexprint(header), len(header)))
        raise errors.RadioError("Header short read")
    if header[0] != 0xAB or header[1] != 0xCD or header[3] != 0x00:
        LOG.warning("Bad response header: %s len=%i" %
                    (util.hexprint(header), len(header)))
        raise errors.RadioError("Bad response header")

    cmd = serport.read(int(header[2]))
    if len(cmd) != int(header[2]):
        LOG.warning("Body short read: [%s] len=%i" %
                    (util.hexprint(cmd), len(cmd)))
        raise errors.RadioError("Command body short read")

    footer = serport.read(4)

    if len(footer) != 4:
        LOG.warning("Footer short read: [%s] len=%i" %
                    (util.hexprint(footer), len(footer)))
        raise errors.RadioError("Footer short read")

    if footer[2] != 0xDC or footer[3] != 0xBA:
        LOG.debug(
                "Reply before bad response footer (obfuscated)"
                "len=0x%4.4x:\n%s" % (len(cmd), util.hexprint(cmd)))
        LOG.warning("Bad response footer: %s len=%i" %
                    (util.hexprint(footer), len(footer)))
        raise errors.RadioError("Bad response footer")

    if DEBUG_SHOW_OBFUSCATED_COMMANDS:
        LOG.debug("Received reply (obfuscated) len=0x%4.4x:\n%s" %
                  (len(cmd), util.hexprint(cmd)))

    cmd2 = xorarr(cmd)

    LOG.debug("Received reply (unobfuscated) len=0x%4.4x:\n%s" %
              (len(cmd2), util.hexprint(cmd2)))

    return cmd2

# --------------------------------------------------------------------------------
def _getstring(data: bytes, begin, maxlen):
    tmplen = min(maxlen+1, len(data))
    s = [data[i] for i in range(begin, tmplen)]
    for key, val in enumerate(s):
        if val < ord(' ') or val > ord('~'):
            break
    return ''.join(chr(x) for x in s[0:key])

# --------------------------------------------------------------------------------
def _sayhello(serport):
    hellopacket = b"\x14\x05\x04\x00\x6a\x39\x57\x64"

    tries = 5
    while True:
        LOG.debug("Sending hello packet")
        _send_command(serport, hellopacket)
        o = _receive_reply(serport)
        if (o):
            break
        tries -= 1
        if tries == 0:
            LOG.warning("Failed to initialise radio")
            raise errors.RadioError("Failed to initialize radio")
    firmware = _getstring(o, 4, 18)
    LOG.info("Found firmware: %s" % firmware)
    return firmware

# --------------------------------------------------------------------------------
def _readmem(serport, offset, length):
    LOG.debug("Sending readmem offset=0x%4.4x len=0x%4.4x" % (offset, length))

    readmem = b"\x1b\x05\x08\x00" + \
        struct.pack("<HBB", offset, length, 0) + \
        b"\x6a\x39\x57\x64"
    _send_command(serport, readmem)
    o = _receive_reply(serport)
    if DEBUG_SHOW_MEMORY_ACTIONS:
        LOG.debug("readmem Received data len=0x%4.4x:\n%s" %
                  (len(o), util.hexprint(o)))
    return o[8:]

# --------------------------------------------------------------------------------
def _writemem(serport, data, offset):
    LOG.debug("Sending writemem offset=0x%4.4x len=0x%4.4x" %
              (offset, len(data)))

    if DEBUG_SHOW_MEMORY_ACTIONS:
        LOG.debug("writemem sent data offset=0x%4.4x len=0x%4.4x:\n%s" %
                  (offset, len(data), util.hexprint(data)))

    dlen = len(data)
    writemem = b"\x1d\x05" + \
        struct.pack("<BBHBB", dlen+8, 0, offset, dlen, 1) + \
        b"\x6a\x39\x57\x64"+data

    _send_command(serport, writemem)
    o = _receive_reply(serport)

    LOG.debug("writemem Received data: %s len=%i" % (util.hexprint(o), len(o)))

    if (o[0] == 0x1e
            and
            o[4] == (offset & 0xff)
            and
            o[5] == (offset >> 8) & 0xff):
        return True
    else:
        LOG.warning("Bad data from writemem")
        raise errors.RadioError("Bad response to writemem")

# --------------------------------------------------------------------------------
def _resetradio(serport):
    resetpacket = b"\xdd\x05\x00\x00"
    _send_command(serport, resetpacket)

# --------------------------------------------------------------------------------
def do_download(radio):
    serport = radio.pipe
    serport.timeout = 0.5
    status = chirp_common.Status()
    status.cur = 0
    status.max = MEM_SIZE
    status.msg = "Downloading from radio"
    radio.status_fn(status)

    eeprom = b""
    f = _sayhello(serport)
    if f:
        radio.FIRMWARE_VERSION = f
    else:
        raise errors.RadioError('Unable to determine firmware version')

    addr = 0
    while addr < MEM_SIZE:
        o = _readmem(serport, addr, MEM_BLOCK)
        status.cur = addr
        radio.status_fn(status)

        if o and len(o) == MEM_BLOCK:
            eeprom += o
            addr += MEM_BLOCK
        else:
            raise errors.RadioError("Memory download incomplete")

    return memmap.MemoryMapBytes(eeprom)

# --------------------------------------------------------------------------------
def do_upload(radio):
    serport = radio.pipe
    serport.timeout = 0.5
    status = chirp_common.Status()
    status.cur = 0
    status.max = PROG_SIZE
    status.msg = "Uploading VFO Setting to radio"
    radio.status_fn(status)

    f = _sayhello(serport)
    if f:
        radio.FIRMWARE_VERSION = f
    else:
        return False
# ---------------Setting 1  
    addr = 0
    while addr < PROG_SIZE_V:
        o = radio.get_mmap()[addr:addr+MEM_BLOCK]
        _writemem(serport, o, addr)
        status.cur = addr
        radio.status_fn(status)
        if o:
            addr += MEM_BLOCK
        else:
            raise errors.RadioError("Upload VFO incomplete")
    status.msg = "Uploading User Setting to radio"
# ---------------Setting 2 
    addr = PROG_SIZE_U
    while addr < PROG_SIZE:
        o = radio.get_mmap()[addr:addr+MEM_BLOCK]
        _writemem(serport, o, addr)
        status.cur = addr
        radio.status_fn(status)
        if o:
            addr += MEM_BLOCK
        else:
            raise errors.RadioError("Upload User Setting incomplete")
    status.msg = "Uploading Memory to radio"    
# ----------------
    status.max = PROG_SIZEM
    status.cur = 0
    addr = START_MEM
    
    while addr < PROG_SIZEM:
        o = radio.get_mmap()[addr:addr+MEM_BLOCK]
        _writemem(serport, o, addr)
        status.cur = addr
        radio.status_fn(status)
        if o:
            addr += MEM_BLOCK
        else:
            raise errors.RadioError("Memory upload incomplete")
    status.msg = "Uploaded  OK"

    _resetradio(serport)

    return True


## #######################################################################################

@directory.register
class UVKxRadio(chirp_common.CloneModeRadio):
    """Quansheng UV-Kx (joaquim.org)"""
    VENDOR = "Quansheng"
    MODEL = "UV-Kx"
    VARIANT = "joaquim.org"
    BAUD_RATE = 115200
    NEEDS_COMPAT_SERIAL = False
    FIRMWARE_VERSION = ""
    _cal_start = 0x1E00  # calibration memory start address
    _steps = [2.5, 5, 6.25, 10, 12.5, 25, 8.33, 0.01, 0.05, 0.1, 0.25, 0.5, 1,
              1.25, 9, 15, 20, 30, 50, 100, 125, 200, 250, 500]

    upload_calibration = False

# --------------------------------------------------------------------------------
    def get_prompts(x=None):
        rp = chirp_common.RadioPrompts()
        rp.experimental = _(
            'This is an experimental driver for the Quansheng UV-K5. '
            'It may harm your radio, or worse. Use at your own risk.\n\n'
            'Before attempting to do any changes please download '
            'the memory image from the radio with chirp '
            'and keep it. This can be later used to recover the '
            'original settings. \n\n'
            'some details are not yet implemented')
        rp.pre_download = _(
            "1. Turn radio on.\n"
            "2. Connect cable to mic/spkr connector.\n"
            "3. Make sure connector is firmly connected.\n"
            "4. Click OK to download image from device.\n\n"
            "It will may not work if you turn on the radio "
            "with the cable already attached\n")
        rp.pre_upload = _(
            "1. Turn radio on.\n"
            "2. Connect cable to mic/spkr connector.\n"
            "3. Make sure connector is firmly connected.\n"
            "4. Click OK to upload the image to device.\n\n"
            "It will may not work if you turn on the radio "
            "with the cable already attached")
        return rp    
# --------------------------------------------------------------------------------    # Return information about this radio's features, including
    # how many memories it has, what bands it supports, etc
    def get_features(self):
        rf = chirp_common.RadioFeatures()
        rf.has_bank = False
        rf.has_rx_dtcs = True
        rf.has_ctone = True
        rf.has_settings = True
        rf.has_comment = False

        rf.valid_dtcs_codes = DTCS_CODES
        rf.valid_name_length = 10
        rf.valid_power_levels = UVK5_POWER_LEVELS
        #rf.valid_special_chans = list(SPECIALS.keys())
        rf.valid_duplexes = ["", "-", "+", "off", "split"]
        rf.valid_tuning_steps = STEP_TABLE_KHZ
        rf.valid_tmodes = ["", "Tone", "TSQL", "DTCS", "Cross"]
        rf.valid_cross_modes = ["Tone->Tone", "Tone->DTCS", "DTCS->Tone","->Tone", "->DTCS", "DTCS->", "DTCS->DTCS"]
        rf.valid_characters = chirp_common.CHARSET_ASCII
        rf.valid_modes = ["FM", "AM", "USB", "CW", "WFM"]
        rf.valid_skips = ["", "S"]
        rf._expanded_limits = True

        # This radio supports memories 1-200 / 1-999
        rf.memory_bounds = (1, CHAN_MAX)

        rf.valid_bands = [(int(18e6), int(1300e6))]
        return rf
# --------------------------------------------------------------------------------
    # Do a download of the radio from the serial port
    def sync_in(self):
        self._mmap = do_download(self)
        self.process_mmap()
# --------------------------------------------------------------------------------
    # Do an upload of the radio to the serial port
    def sync_out(self):
        do_upload(self)
# --------------------------------------------------------------------------------
    # Convert the raw byte array into a memory object structure
    def process_mmap(self):
        self._memobj = bitwise.parse(MEM_FORMAT, self._mmap)
# -------------------------------------------------------------------------------- 
    # Return a raw representation of the memory object, which
    # is very helpful for development
    def get_raw_memory(self, number):
        return repr(self._memobj.channel[number-1])
    

## #######################################################################################

# --------------------------------------------------------------------------------
    def _verify_channel_number(self, number):
        if number < 1 or number > CHAN_MAX:
            raise errors.InvalidMemoryLocation("Channel %s out of range" %
                                               number)

# --------------------------------------------------------------------------------
    def _channel_struct(self, number):
        self._verify_channel_number(number)
        return self._memobj.channel[number-1]

# --------------------------------------------------------------------------------
    def _extract_name(self, field):
        name = ""
        for char in field:
            if str(char) in ("\x00", "\xff"):
                break
            name += str(char)
        return name.strip()

# --------------------------------------------------------------------------------
    def _encode_name_value(self, name):
        clean = (name or "").strip()
        clean = clean[:10]
        return clean.ljust(10, "\x00")

# --------------------------------------------------------------------------------
    def _decode_freq(self, raw_value):
        return int(raw_value) * FREQ_UNITS

# --------------------------------------------------------------------------------
    def _encode_freq(self, hz):
        hz = int(round(hz))
        value = int(round(hz / FREQ_UNITS))
        value = max(0, min(MAX_FREQ_VALUE, value))
        return value

# --------------------------------------------------------------------------------
    def _mode_from_value(self, value):
        return MODE_MAP.get(int(value), "FM")

# --------------------------------------------------------------------------------
    def _mode_to_value(self, mode):
        if mode is None:
            return 0
        mode = mode.upper()
        if mode == "AM":
            return 1
        if mode == "USB":
            return 3
        if mode in ("LSB", "CW"):
            return 2
        if mode == "WFM":
            return 6
        return 0

# --------------------------------------------------------------------------------
    def _step_from_value(self, value):
        if 0 <= value < len(STEP_TABLE_KHZ):
            return STEP_TABLE_KHZ[value]
        return STEP_TABLE_KHZ[0]

# --------------------------------------------------------------------------------
    def _step_to_index(self, step):
        if step is None:
            return 0
        closest = min(range(len(STEP_TABLE_KHZ)),
                      key=lambda idx: abs(STEP_TABLE_KHZ[idx] - step))
        return closest

# --------------------------------------------------------------------------------
    def _power_from_value(self, value):
        idx = int(value)
        idx = max(0, min(len(UVK5_POWER_LEVELS) - 1, idx))
        return UVK5_POWER_LEVELS[idx]

# --------------------------------------------------------------------------------
    def _power_to_value(self, power):
        if not power:
            return 0
        for idx, level in enumerate(UVK5_POWER_LEVELS):
            if str(level) == str(power):
                return idx
        return 0

# --------------------------------------------------------------------------------
    def _channel_is_empty(self, channel):
        return int(channel.rx_frequency) == 0

# --------------------------------------------------------------------------------
    def _clear_channel(self, channel):
        channel.rx_frequency = 0
        channel.rx_code_type = CODETYPE_NONE
        channel.rx_code = 0
        channel.tx_frequency = 0
        channel.tx_code_type = CODETYPE_NONE
        channel.tx_code = 0
        channel.name = "\x00" * 10
        channel.channel_id = 0
        channel.squelch = 0
        channel.step = 0
        channel.modulation = 0
        channel.bandwidth = 0
        channel.power = 0
        channel.shift = SHIFT_NONE
        channel.repeater_ste = 0
        channel.ste = 0
        channel.compander = 0
        channel.roger = 0
        channel.pttid = 0
        channel.rxagc = 0

# --------------------------------------------------------------------------------
    def _tone_triplet(self, code_type, code_index):
        code_type = int(code_type)
        code_index = int(code_index)
        if code_type == CODETYPE_TONE:
            if 0 <= code_index < len(CTCSS_TONES):
                return ("Tone", CTCSS_TONES[code_index], "N")
            return ("Tone", None, "N")
        if code_type in (CODETYPE_DTCS, CODETYPE_INV_DTCS):
            if 0 <= code_index < len(DTCS_CODES):
                polarity = "R" if code_type == CODETYPE_INV_DTCS else "N"
                return ("DTCS", DTCS_CODES[code_index], polarity)
            return ("DTCS", None, "N")
        return ("", None, "N")

# --------------------------------------------------------------------------------
    def _decode_tones(self, mem, channel):
        tx_triplet = self._tone_triplet(channel.tx_code_type, channel.tx_code)
        rx_triplet = self._tone_triplet(channel.rx_code_type, channel.rx_code)
        chirp_common.split_tone_decode(mem, tx_triplet, rx_triplet)

# --------------------------------------------------------------------------------
    def _find_ctcss_index(self, tone):
        if tone is None:
            return None
        for idx, value in enumerate(CTCSS_TONES):
            if abs(value - tone) < 0.1:
                return idx
        raise errors.RadioError("Unsupported CTCSS tone %.1f" % tone)

# --------------------------------------------------------------------------------
    def _find_dtcs_index(self, code):
        if code is None:
            return None
        code = int(code)
        if code in DTCS_CODES:
            return DTCS_CODES.index(code)
        raise errors.RadioError("Unsupported DTCS code %i" % code)

# --------------------------------------------------------------------------------
    def _set_tone(self, mem, channel):
        ((tx_mode, tx_tone, tx_pol),
         (rx_mode, rx_tone, rx_pol)) = chirp_common.split_tone_encode(mem)

        tx_type = CODETYPE_NONE
        tx_code = 0
        if tx_mode == "Tone" and tx_tone is not None:
            tx_type = CODETYPE_TONE
            tx_code = self._find_ctcss_index(tx_tone)
        elif tx_mode == "DTCS" and tx_tone is not None:
            tx_type = CODETYPE_INV_DTCS if tx_pol == "R" else CODETYPE_DTCS
            tx_code = self._find_dtcs_index(tx_tone)

        rx_type = CODETYPE_NONE
        rx_code = 0
        if rx_mode == "Tone" and rx_tone is not None:
            rx_type = CODETYPE_TONE
            rx_code = self._find_ctcss_index(rx_tone)
        elif rx_mode == "DTCS" and rx_tone is not None:
            rx_type = CODETYPE_INV_DTCS if rx_pol == "R" else CODETYPE_DTCS
            rx_code = self._find_dtcs_index(rx_tone)

        channel.tx_code_type = tx_type
        channel.tx_code = tx_code or 0
        channel.rx_code_type = rx_type
        channel.rx_code = rx_code or 0

# --------------------------------------------------------------------------------
    def _decode_duplex(self, channel, rx_hz):
        tx_hz = self._decode_freq(channel.tx_frequency)
        shift = int(channel.shift)
        if tx_hz == 0:
            return "off", 0
        if shift == SHIFT_PLUS:
            return "+", abs(tx_hz - rx_hz)
        if shift == SHIFT_MINUS:
            return "-", abs(rx_hz - tx_hz)
        if tx_hz != rx_hz:
            return "split", tx_hz
        return "", 0

# --------------------------------------------------------------------------------
    def _apply_duplex(self, mem, channel, rx_units):
        duplex = (mem.duplex or "").lower()
        if duplex == "+":
            offset_units = self._encode_freq(mem.offset or 0)
            channel.tx_frequency = min(MAX_FREQ_VALUE, rx_units + offset_units)
            channel.shift = SHIFT_PLUS
        elif duplex == "-":
            offset_units = self._encode_freq(mem.offset or 0)
            channel.tx_frequency = max(0, rx_units - offset_units)
            channel.shift = SHIFT_MINUS
        elif duplex == "off":
            channel.tx_frequency = 0
            channel.shift = SHIFT_NONE
        elif duplex == "split":
            channel.tx_frequency = self._encode_freq(mem.offset or 0)
            channel.shift = SHIFT_NONE
        else:
            channel.tx_frequency = rx_units
            channel.shift = SHIFT_NONE


## #######################################################################################

# --------------------------------------------------------------------------------
    # Extract a high-level memory object from the low-level memory map
    # This is called to populate a memory in the UI
    def get_memory(self, number2):

        mem = chirp_common.Memory()

        self._verify_channel_number(number2)
        mem.number = number2

        channel = self._channel_struct(number2)

        if self._channel_is_empty(channel):
            mem.empty = True
            mem.name = ""
            return mem

        mem.empty = False
        mem.name = self._extract_name(channel.name)
        mem.freq = self._decode_freq(channel.rx_frequency)
        mem.duplex, mem.offset = self._decode_duplex(channel, mem.freq)
        # The EEPROM stores modulation in the low nibble and bandwidth in the high
        # nibble of the same byte; bitwise parses the first field into the high
        # nibble. Swap them here to get the real values.
        parsed_mod = int(channel.bandwidth)
        parsed_bw = int(channel.modulation)

        mem.mode = self._mode_from_value(parsed_mod)
        # Step nibble is stored high; bitwise swaps it with squelch.
        parsed_step = int(channel.squelch)
        parsed_squelch = int(channel.step)
        mem.tuning_step = self._step_from_value(parsed_step)
        mem.power = self._power_from_value(channel.power)
        self._decode_tones(mem, channel)

        # Expose bandwidth in the per-memory extras so CHIRP can edit it
        bw_index = int(min(len(BANDWIDTH_OPTIONS) - 1, max(0, parsed_bw)))
        mem.extra = RadioSettingGroup("extra", "Extra")
        mem.extra.append(
            RadioSetting(
                "bandwidth",
                "Bandwidth (kHz)",
                RadioSettingValueList(BANDWIDTH_OPTIONS, current_index=bw_index),
            )
        )
        mem.extra.append(
            RadioSetting(
                "squelch",
                "Squelch (0-9)",
                RadioSettingValueInteger(0, 9, parsed_squelch),
            )
        )
        rxagc_index = int(min(len(RXAGC_OPTIONS) - 1, max(0, int(channel.rxagc))))
        mem.extra.append(
            RadioSetting(
                "rxagc",
                "RX AGC (dB)",
                RadioSettingValueList(RXAGC_OPTIONS, current_index=rxagc_index),
            )
        )

        return mem
    

## #######################################################################################

# --------------------------------------------------------------------------------
    # Store details about a high-level memory to the memory map
    # This is called when a user edits a memory in the UI
    def set_memory(self, mem):

        self._verify_channel_number(mem.number)
        channel = self._channel_struct(mem.number)

        if mem.empty:
            self._clear_channel(channel)
            return

        if not mem.freq:
            raise errors.RadioError("Frequency must be set")

        channel.name = self._encode_name_value(mem.name)
        channel.channel_id = mem.number

        rx_units = self._encode_freq(mem.freq)
        channel.rx_frequency = rx_units

        self._apply_duplex(mem, channel, rx_units)

        desired_mod = self._mode_to_value(mem.mode)
        bw_index = int(channel.bandwidth)
        if mem.extra:
            for setting in mem.extra:
                if isinstance(setting, RadioSetting) and setting.get_name() == "bandwidth":
                    try:
                        bw_index = int(setting.value)
                    except Exception:
                        pass
                    break

        # Preserve squelch nibble (low) before overwriting fields
        stored_squelch = int(channel.step)
        desired_step_index = self._step_to_index(mem.tuning_step or STEP_TABLE_KHZ[0])
        desired_squelch = stored_squelch
        if mem.extra:
            for setting in mem.extra:
                if isinstance(setting, RadioSetting) and setting.get_name() == "squelch":
                    try:
                        desired_squelch = int(setting.value)
                    except Exception:
                        pass
                    break

        if mem.extra:
            for setting in mem.extra:
                if isinstance(setting, RadioSetting) and setting.get_name() == "rxagc":
                    try:
                        channel.rxagc = int(setting.value)
                    except Exception:
                        pass
                    break

        # Write back swapped to match EEPROM layout (modulation low nibble, bandwidth high nibble)
        channel.modulation = min(len(BANDWIDTH_OPTIONS) - 1, max(0, bw_index))
        channel.bandwidth = desired_mod
        channel.squelch = desired_step_index
        channel.step = desired_squelch
        channel.power = self._power_to_value(mem.power)

        self._set_tone(mem, channel)

        return mem
    


## #######################################################################################
## #######################################################################################

#--------------------------------------------------------------------------------
    def get_settings(self):
        
        settings = self._memobj.settings

        radio = RadioSettingGroup("radio", _("Radio Settings"))
        display = RadioSettingGroup("display", _("Display"))
        roinfo = RadioSettingGroup("roinfo", _("Driver information"))

        battery_index = min(len(BATTERY_TYPES) - 1, int(settings.battery_type))
        tx_timeout_index = min(len(TX_TIMEOUT_OPTIONS) - 1,
                               int(settings.tx_timeout))
        mic_index = max(0, min(len(MIC_DB_OPTIONS) - 1,
                               int(settings.mic_gain_db) - 1))
        backlight_time_index = min(len(BACKLIGHT_TIME_OPTIONS) - 1,
                                   int(settings.backlight_time))
        backlight_mode_index = min(len(BACKLIGHT_MODE_OPTIONS) - 1,
                                   int(settings.backlight_mode))
        contrast_index = min(len(LCD_CONTRAST_OPTIONS) - 1,
                             int(settings.lcd_contrast))

        radio.append(
            RadioSetting(
                "battery_type",
                _("Battery Type"),
                RadioSettingValueList(BATTERY_TYPES,
                                      current_index=battery_index),
            )
        )

        radio.append(
            RadioSetting(
                "busy_lockout",
                _("Busy Lockout"),
                RadioSettingValueBoolean(bool(settings.busy_lockout)),
            )
        )

        radio.append(
            RadioSetting(
                "beep",
                _("Keypad Beep"),
                RadioSettingValueBoolean(bool(settings.beep)),
            )
        )

        radio.append(
            RadioSetting(
                "battery_save",
                _("Battery Save"),
                RadioSettingValueBoolean(bool(settings.battery_save)),
            )
        )

        radio.append(
            RadioSetting(
                "tx_timeout",
                _("TX Time-Out Timer"),
                RadioSettingValueList(TX_TIMEOUT_OPTIONS,
                                      current_index=tx_timeout_index),
            )
        )

        radio.append(
            RadioSetting(
                "mic_gain_db",
                _("Mic Gain"),
                RadioSettingValueList(MIC_DB_OPTIONS,
                                      current_index=mic_index),
            )
        )

        display.append(
            RadioSetting(
                "backlight_level",
                _("Backlight Level"),
                RadioSettingValueInteger(0, 15, int(settings.backlight_level)),
            )
        )

        display.append(
            RadioSetting(
                "backlight_time",
                _("Backlight Time"),
                RadioSettingValueList(BACKLIGHT_TIME_OPTIONS,
                                      current_index=backlight_time_index),
            )
        )

        display.append(
            RadioSetting(
                "backlight_mode",
                _("Backlight Mode"),
                RadioSettingValueList(BACKLIGHT_MODE_OPTIONS,
                                      current_index=backlight_mode_index),
            )
        )

        display.append(
            RadioSetting(
                "lcd_contrast",
                _("LCD Contrast"),
                RadioSettingValueList(LCD_CONTRAST_OPTIONS,
                                      current_index=contrast_index),
            )
        )

        display.append(
            RadioSetting(
                "vfo_selected",
                _("Active VFO"),
                RadioSettingValueList(["VFO A", "VFO B"],
                                      current_index=min(1, int(settings.vfo_selected))),
            )
        )

        display.append(
            RadioSetting(
                "show_vfo_a",
                _("Show VFO A"),
                RadioSettingValueBoolean(bool(settings.show_vfo[0])),
            )
        )

        display.append(
            RadioSetting(
                "show_vfo_b",
                _("Show VFO B"),
                RadioSettingValueBoolean(bool(settings.show_vfo[1])),
            )
        )

        version = "%04X" % int(settings.version)
        version_setting = RadioSetting(
            "settings_version",
            _("Settings Version"),
            RadioSettingValueString(4, 4, version, autopad=False),
        )
        version_setting.value.set_mutable(False)
        roinfo.append(version_setting)

        top = RadioSettings(radio, display, roinfo)

        return top
    

#--------------------------------------------------------------------------------
    def set_settings(self, settings):

        _mem = self._memobj.settings

        for element in settings:
            if not isinstance(element, RadioSetting):
                self.set_settings(element)
                continue

            name = element.get_name()
            if name == "battery_type":
                _mem.battery_type = int(element.value)
            elif name == "busy_lockout":
                _mem.busy_lockout = int(bool(element.value))
            elif name == "beep":
                _mem.beep = int(bool(element.value))
            elif name == "battery_save":
                _mem.battery_save = int(bool(element.value))
            elif name == "tx_timeout":
                _mem.tx_timeout = int(element.value)
            elif name == "mic_gain_db":
                _mem.mic_gain_db = int(element.value) + 1
            elif name == "backlight_level":
                _mem.backlight_level = int(element.value)
            elif name == "backlight_time":
                _mem.backlight_time = int(element.value)
            elif name == "backlight_mode":
                _mem.backlight_mode = int(element.value)
            elif name == "lcd_contrast":
                _mem.lcd_contrast = int(element.value)
            elif name == "vfo_selected":
                _mem.vfo_selected = int(element.value)
            elif name == "show_vfo_a":
                _mem.show_vfo[0] = int(bool(element.value))
            elif name == "show_vfo_b":
                _mem.show_vfo[1] = int(bool(element.value))
            else:
                LOG.debug("Unknown setting: %s" % name)
