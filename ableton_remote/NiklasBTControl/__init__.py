from ableton.v3.control_surface import ControlSurface, ControlSurfaceSpecification, create_skin  # Import v3 ControlSurface base classes
from ableton.v3.control_surface.capabilities import (CONTROLLER_ID_KEY, PORTS_KEY, NOTES_CC, REMOTE, SCRIPT, controller_id, inport, outport)  # Import v3 capabilities

# --- Import custom modules --- #
from .component import Buttons  # Import Buttons component
from .elements import Elements  # Import Elements class
from .logger import Logger      # Import custom logger

# --- create_mappings and Specification --- #
def create_mappings(control_surface):  # Function to define control mappings
  return {
    "Buttons": dict(  # Map all button controls to their element names
      loop_button="loop_button",
      rwd_cue_button="rwd_cue_button",
      fwd_cue_button="fwd_cue_button",
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
  component_map = {"Buttons": Buttons}  # Register Buttons component


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
    Logger.info("Got midi?")  # Log script startup
    
    try: # Add listener for arranger loop state
      self.song.add_loop_listener(self._on_loop_changed)  # Listen for loop changes
    except Exception as e: Logger.error(f"Failed to add loop listener: {e}")  # Log errors

  def _on_loop_changed(self):  # Callback for arranger loop changes
    try:
      self._c_instance.send_midi((0xB0, 100, 127 if self.song.loop else 0))  # Send MIDI feedback to controller based on loop state
      Logger.info(f"Loop = {'ON' if self.song.loop else 'OFF'}")  # Log feedback
    except Exception as e: Logger.error(f"Failed to send loop feedback MIDI: {e}")  # Log errors

  def disconnect(self):  # Called when script is unloaded
    Logger.info("Hej då!")  # Log shutdown
    try:
      self.song.remove_loop_listener(self._on_loop_changed)  # Remove loop listener
    except Exception: pass  # Ignore errors
    super().disconnect()  # Call base class disconnect


# --- create_instance --- #
def create_instance(c_instance):  # Entry point for Ableton
  return NiklasBTControl(Specification, c_instance=c_instance)  # Return script instance
