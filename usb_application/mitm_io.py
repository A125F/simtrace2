import usb.core
import usb.util
import array

from ccid_raw import SmartcardConnection
from smartcard_emulator import SmartCardEmulator
from gsmtap import gsmtap_send_apdu

from contextlib import closing

from util import HEX
from constants import *
from apdu_split import Apdu_splitter, apdu_states

def pattern_match(inpt):
    print("Matching inpt", inpt)
    if (inpt == ATR_SYSMOCOM1) or (inpt == ATR_STRANGE_SIM):
        print("ATR: ", inpt)
        return NEW_ATR
    elif (inpt == CMD_SEL_FILE):
        print("CMD_SEL_FILE:", inpt)
        return CMD_SEL_ROOT
    elif (inpt == CMD_GET_DATA):
        print("CMD_DATA:", inpt)
        return CMD_SEL_ROOT
    else:
        return inpt

def poll_ep(dev, ep):
    try:
        return dev.read(ep, 64, 10)
    except usb.core.USBError as e:
        if e.errno != ERR_TIMEOUT:
            raise
        return None

def write_phone(dev, resp):
    print("WR: ", HEX(resp))
    dev.write(PHONE_WR, resp, 10)

def do_mitm(dev, sim_emul=True):
    if sim_emul == True:
        my_class = SmartCardEmulator
    else:
        my_class = SmartcardConnection
    with closing(my_class()) as sm_con:
        atr = sm_con.getATR()

        apdus = []
        apdu = Apdu_splitter()

        mitm = MitM()

        while True:
            cmd = poll_ep(dev, PHONE_INT)
            if cmd is not None:
                print("Int line ", HEX(cmd))
                assert cmd[0] == ord('R')
# FIXME: restart card anyways?
#               sm_con.reset_card()
                print("Write atr: ", HEX(atr))
                write_phone(dev, mitm.attack(ATR, (atr)))
                apdus = []
                apdu = Apdu_splitter()

            cmd = poll_ep(dev, PHONE_RD)
            if cmd is not None:
                print("RD: ", HEX(cmd))
                for c in cmd:
                    if apdu.state == apdu_states.APDU_S_FIN:
                        apdus.append(apdu)
                        gsmtap_send_apdu(apdu.buf)
                        apdu = Apdu_splitter()

                    apdu.split(c)
                    if apdu.state == apdu_states.APDU_S_FIN and apdu.pts_buf == [0xff, 0x00, 0xff]:
                        write_phone(dev,  mitm.attack(PTS_BUF, array('B', apdu.pts_buf)))
                        continue;

                    if apdu.state == apdu_states.APDU_S_SW1:
                        if apdu.data is not None and len(apdu.data) == 0:
                            # FIXME: implement other ACK types
                            write_phone(dev,  mitm.attack(APDU_INS, (array('B', [apdu.ins]))))
                            apdu.split(apdu.ins)
                        else:
                            sim_data = sm_con.send_receive_cmd(apdu.buf)
                            write_phone(dev, mitm.attack(SIM_DATA, (sim_data)))
                            for c in sim_data:
                                apdu.split(c)
                    if apdu.state == apdu_states.APDU_S_SEND_DATA:
                        sim_data = sm_con.send_receive_cmd(apdu.buf)
                        #sim_data.insert(0, apdu.ins)
                        write_phone(dev, mitm.attack(SIM_DATA, (sim_data)))
                        #apdu.state = apdu_states.APDU_S_SW1
                        for c in sim_data:
                            apdu.split(c)