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
  
    self._last_clip = None  # Initialize last clip variable
    
    try: self.song.add_current_song_time_listener(self._on_song_time_changed) # Add listener for time
    except Exception as e: Logger.error(f"Hey, listen! Failed to add time listener: {e}") # Log errors
    
    try: self.song.add_loop_listener(self._on_loop_changed) # Add listener for arranger loop state
    except Exception as e: Logger.error(f"Hey, listen! Failed to add loop listener: {e}") # Log errors
    
    try: self.schedule_message(3000, self._post_init) # Schemalägg ett "sista" MIDI-meddelande efter 3000 ms / 3 sekunder
    except Exception as e: Logger.error(f"Failed to schedule: {e}")
    
    Logger.info("Got midi?")  # Log script startupa
    self.show_message(f"Got midi?")

  def _post_init(self):
    try:
      Logger.info("Init loop-status meddelande.") # Log initial MIDI feedback
      self._c_instance.send_midi((0xB0, 0x64, 0x7F if self.song.loop else 0x00)) # Send initial MIDI feedback based on loop state
    except Exception as e: Logger.error(f"MIDI: {e}")

  def _set_follow(self):
    try: self.song.view.follow_song = False if self.song.loop else True
    except Exception as e: Logger.error(f"Failed to set follow_song: {e}")  # Log errors if setting follow fails
        
  def _set_auto_loop(self, time):
    try:
      start, lenght = time
      self.song.loop_start = start
      self.song.loop_length = lenght
      self.song.loop = True # Enable loop
      Logger.info(f"Loop set!")
      self.show_message(f"Autoloop!")
    except Exception as e: Logger.error(f"Failed to set loop: {e}") # Log errors if setting loop fails
    
  def _on_song_time_changed(self):
    if len(self.song.tracks) > 0: # Check if there are any tracks
      if hasattr(self.song.tracks[0], "name") and self.song.tracks[0].name == "INFO": # Check if the track is named "INFO"
        if hasattr(self.song.tracks[0], "arrangement_clips"): # Check if the track has arrangement clips
          for _clip in self.song.tracks[0].arrangement_clips: # Iterate through arrangement clips
            if _clip.start_time <= self.song.current_song_time < (_clip.start_time + _clip.length): # Check if current time is within clip bounds
              if _clip.color_index == 3 or _clip.color_index == 17: # Check if clip color is either 3 or 17
                if self._last_clip != _clip: # If the last clip is not the same as the current clip
                  try: 
                    self.schedule_message(1, self._set_auto_loop, (_clip.start_time, _clip.length)) # Set auto loop with clip start and end time
                    self._last_clip = _clip # Update last clip
                  except Exception as e: Logger.error(f"Oh no: {e}")
                return
              

  def _on_loop_changed(self):  # Callback for arranger loop changes
    try:
      self._c_instance.send_midi((0xB0, 0x64, 0x7F if self.song.loop else 0x00)) # Send MIDI feedback to controller based on loop state
      self.schedule_message(1, self._set_follow)  # Schedule follow state updaten loop
    except Exception as e: Logger.error(f"Hey, listen! We failed to send feedback MIDI: {e}")  # Log errors

  def disconnect(self):  # Called when script is unloaded
    Logger.info("Hej då!")  # Log shutdown
    try:
      self.song.remove_current_song_time_listener(self._on_song_time_changed)  # Remove time listener
      self.song.remove_loop_listener(self._on_loop_changed)  # Remove loop listener
    except Exception: pass  # Ignore errors
    super().disconnect()  # Call base class disconnect

def create_instance(c_instance):  # Entry point for Ableton
  return NiklasBTControl(Specification, c_instance=c_instance)  # Return script instance
