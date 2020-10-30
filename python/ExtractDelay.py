#!/usr/bin/env python
# 

from gnuradio import gr
import pmt

class ExtractDelay(gr.sync_block):
    """
    This block extracts the delay value for the provided index from the corrective_lags
    meta vector output from the xcorrelate block, and controls a variable.
    """
    def __init__(self, index_to_extract, callback, lock):
        gr.sync_block.__init__(self, name="ExtractDelay", in_sig=None, out_sig=None)

        self.index_to_extract = index_to_extract
        self.callback = callback
        self.lock = lock

        self.message_port_register_in(pmt.intern("corr"))
        self.set_msg_handler(pmt.intern("corr"), self.msg_handler)
        self.message_port_register_out(pmt.intern("delay"))

    def msg_handler(self, msg):
        if self.lock:
            return
            
        if not pmt.is_pair(msg):
            gr.log.warn("Incoming message is not a pair.  Only pairs are supported.  "
                        "No message generated.")
            return

        meta = pmt.to_python(pmt.car(msg))

        if not type(meta) is dict:
            gr.log.warn("Incoming message does not contain a dictionary.  "
                        "No message generated.")
            return

        if not 'corrective_lags' in meta:
            gr.log.warn("Incoming message dictionary does not contain key corrective_lags.  "
                        "No message generated.")
            return

        try:
            new_delay = int(meta['corrective_lags'][self.index_to_extract])
        except Exception as e:
            gr.log.error("Cannot extract lag info: %s" % str(e))
            return

        self.callback(new_delay)

        new_pair = None

        try:
            new_pair = pmt.cons(pmt.intern('delay'), pmt.to_pmt(new_delay))
        except Exception as e:
            gr.log.error("Cannot construct new message: %s" % str(e))
            return

        try:
            self.message_port_pub(pmt.intern("delay"), new_pair)
        except Exception as e:
            gr.log.error("Cannot send message: %s" % str(e))
            gr.log.error("Incoming dictionary (%s):" % str(type(meta)))
            gr.log.error(str(meta))

    def set_lock(self, lock):
        self.lock = lock

    def stop(self):
        return True
