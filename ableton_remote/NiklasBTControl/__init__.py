from ableton.v3.control_surface import ControlSurface, ControlSurfaceSpecification, create_skin  # Import v3 ControlSurface base classes
from ableton.v3.control_surface.capabilities import (CONTROLLER_ID_KEY, PORTS_KEY, NOTES_CC, REMOTE, SCRIPT, controller_id, inport, outport)  # Import v3 capabilities

# --- Import custom modules --- #
from .component import Transport  # Import Transport component
from .component import Buttons  # Import Buttons component
from .elements import Elements  # Import Elements class
from .logger import Logger      # Import custom logger

# --- create_mappings and Specification --- #
def create_mappings(control_surface): # Function to define control mappings
  return {
    "Transport": dict(  # Map transport controls to their element names
      loop_button="loop_button",  # Map loop button to its element name
      prev_cue_button="rwd_cue_button",
      next_cue_button="fwd_cue_button",
    ),
    "Buttons": dict( # Map all button controls to their element names
      rwd_bar_button="rwd_bar_button",
      fwd_bar_button="fwd_bar_button",
      rwd_bit_button="rwd_bit_button",
      fwd_bit_button="fwd_bit_button"
    ),
  }
  
# --- Specification class with custom elements and mappings --- #
class Specification(ControlSurfaceSpecification):  # Define the script specification
  elements_type = Elements  # Use our "custom" Elements class
  control_surface_skin = create_skin()  # Use empty skin which is created here...
  create_mappings_function = create_mappings  # Use our mapping function created above
  component_map = {
    "Transport": Transport,
    "Buttons": Buttons # Register Buttons component
  }

def get_capabilities():  # Define script capabilities for Ableton
  return {    
    CONTROLLER_ID_KEY: controller_id(
      vendor_id=2000,  # Vendor ID for the controller
      product_ids=[2000],  # Product IDs for the controller
      model_name=["Niklas BTControl"],  # Model name for the controller
    ),
    PORTS_KEY: [
      inport(props=[NOTES_CC, SCRIPT, REMOTE]),  # Input port properties
      outport(props=[SCRIPT]),  # Output port properties
    ],
  }

# --- Main ControlSurface --- #
class NiklasBTControl(ControlSurface):  # Main script class
  def __init__(self, *a, **k):
    super().__init__(*a, **k)  # Initialize base class
    
    try: # Add listener for arranger loop state
      self.song.add_loop_listener(self._on_loop_changed)
    except Exception as e: Logger.error(f"Hey, listen! Failed to add loop listener: {e}")  # Log errors
    
    Logger.info("Got midi?")  # Log script startupa

    # Schemalägg ett "sista" MIDI-meddelande efter 3000 ms / 3 sekunder
    try:
      self.schedule_message(3000, self._post_init)
    except Exception as e: Logger.error(f"Failed to schedule: {e}")
  
  
    
  def _post_init(self):
    try:
      Logger.info("Init loop-status meddelande.")  # Log initial MIDI feedback
      self._c_instance.send_midi((0xB0, 0x64, 0x7F if self.song.loop else 0x00)) # Send initial MIDI feedback based on loop state
    except Exception as e: Logger.error(f"MIDI: {e}")



  def _on_loop_changed(self):  # Callback for arranger loop changes
    try:
      Logger.info(f"Hey, listen! The arrangement loop-state has changed! The loop is now {'on' if self.song.loop else 'off'}, I'll send MIDI...")  # Log feedback
      self._c_instance.send_midi((0xB0, 0x64, 0x7F if self.song.loop else 0x00)) # Send MIDI feedback to controller based on loop state
    except Exception as e: Logger.error(f"Hey, listen! failed to send loop feedback MIDI: {e}")  # Log errors

    
  # def _play(self):
    
  #   # next_bar = int(self.song.current_song_time // self.song.signature_numerator) + 1 # "RÄKNEFEL"...
  #   # next_bar_time = next_bar * self.song.signature_numerator

  #   # # Trigga kod om vi är mindre än t.ex. 1/96-dels beat från nästa etta
  #   # if 0 < (next_bar_time - self.song.current_song_time) < (1.0 / 96):
  #   #   if self._next_one != next_bar:
  #   #     Logger.info(f"Snart etta! (vid beat {next_bar_time})")
  #   #     # Din kod här, t.ex. skicka MIDI eller annat
  #   #     self._next_one = next_bar
  #   #   else:
  #   #     # Återställ flaggan om vi är långt från nästa etta
  #   #     if self._next_one == next_bar:
  #   #       self._next_one = None
          
    
  #   # def _on_song_time_changed(self):
  #   _live_time = self.song.current_song_time
    
  #   # Första gången: initiera
  #   if not hasattr(self, 'live_prev'):
  #     _live_prev = _live_time
  #     return

  #   # Räkna ut skillnad
  #   _delta = abs(_live_time - _live_prev)
    
  #   # Om hoppet är större än t.ex. 0.75 beats
  #   if _delta > 0.75:
  #     Logger.info(f"Playhead disturbance detected! Δ = {_delta:.2f}")
      
  #     # self.get_component("Buttons.rwd_cue_button").rwd_count = 0
  #     # self.get_component("Buttons").rwd_count = 0
  #     # self.get_component("Buttons").rwd_count = 0
  #     # 
  #     #
  #     #
  #     #
  #     #
  #     #
  #     #

  #   _live_prev = _live_time
            

  def disconnect(self):  # Called when script is unloaded
    Logger.info("Hej då!")  # Log shutdown
    try:
      self.song.remove_loop_listener(self._on_loop_changed)  # Remove loop listener
    except Exception: pass  # Ignore errors
    super().disconnect()  # Call base class disconnect

# --- create_instance --- #
def create_instance(c_instance):  # Entry point for Ableton
  return NiklasBTControl(Specification, c_instance=c_instance)  # Return script instance
