from ableton.v3.control_surface.elements import ButtonElement
from ableton.v3.control_surface import MIDI_CC_TYPE, MIDI_NOTE_TYPE
from .logger import Logger

# --- Elements class with mapped loop_button ---
class Elements:
  def __init__(self, *a, **k):
    
    Logger.info("Elements __init__")
    
    self.loop_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_CC_TYPE,
      channel=0,
      identifier=100,
      name="Loop_Button"
    )
    
    # self.loop_query = ButtonElement(
    #   is_momentary=True,
    #   msg_type=MIDI_NOTE_TYPE,
    #   channel=0,
    #   identifier=0,
    #   name="Loop_Query"
    # )
    
    self.rwd_cue_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=1,
      name="RWD_CUE_button"
    )
    
    self.fwd_cue_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=4,
      name="FWD_CUE_button"
    )
    
    self.rwd_bar_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=2,
      name="RWD_BAR_button"
    )
    
    self.fwd_bar_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=5,
      name="FWD_BAR_button"
    )
    
    self.rwd_bit_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=3,
      name="RWD_BIT_button"
    )
    
    self.fwd_bit_button = ButtonElement(
      is_momentary=True,
      msg_type=MIDI_NOTE_TYPE,
      channel=0,
      identifier=6,
      name="FWD_BIT_button"
    )