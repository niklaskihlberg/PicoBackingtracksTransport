import time
from ableton.v3.control_surface import Component
from ableton.v3.control_surface.controls import ButtonControl
from .logger import Logger

# --- LoopComponent with ButtonControl --- #
class Buttons(Component):
  
  loop_button    = ButtonControl()
  rwd_cue_button = ButtonControl()
  fwd_cue_button = ButtonControl()
  rwd_bar_button = ButtonControl()
  fwd_bar_button = ButtonControl()
  rwd_bit_button = ButtonControl()
  fwd_bit_button = ButtonControl()

  def __init__(self, *a, **k):
    super().__init__(*a, **k)
    self._control_surface = None  # Will be set after instantiation
    self._last_rwd_cue_time = 0
    self._rwd_cue_count = 0
    self._last_bar_checked = None

  @loop_button.pressed
  def loop_button(self, button):
    if getattr(button, "value", 127) == 127:
      Logger.info(f"Loop button pressed (MIDI value 127)!")
      try:
        song = self.song
        # song.loop = not song.loop  # Toggle arranger loop
        # Logger.info(f"Arranger loop: {'ON' if song.loop else 'OFF'}")
        
        if (song.loop):
          song.loop = False
          Logger.info(f"Arranger loop: OFF")
          self._control_surface.send_midi((0xB0, 0x64, 0))
          
        else:
          song.loop = True
          Logger.info(f"Arranger loop: ON")
          self._control_surface.send_midi((0xB0, 0x64, 127))
          
      except Exception as e:
        Logger.error(f"Failed to toggle arranger loop: {e}")
    else:
      Logger.info("Loop button ignored (MIDI value 0)")
    
  @rwd_cue_button.pressed
  def rwd_cue_button(self, _):
    import time
    Logger.info("rwd_cue_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      bar = int(current_pos // song.signature_numerator)
      now = time.time()
      # Nollställ räknare om vi är på en ny takt (ny "etta")
      if self._last_bar_checked is None or bar != self._last_bar_checked:
        self._rwd_cue_count = 1
      elif now - self._last_rwd_cue_time < 0.5:
        self._rwd_cue_count += 1
      else:
        self._rwd_cue_count = 1
      self._last_rwd_cue_time = now
      self._last_bar_checked = bar

      cue_points = sorted(song.cue_points, key=lambda c: c.time)
      
      # Hitta alla cue_points före nuvarande position
      prevs = [cue for cue in cue_points if cue.time < current_pos]
      if prevs:
        idx = max(0, len(prevs) - self._rwd_cue_count)
        prev = prevs[idx]
        prev.jump()
        Logger.info(f"Jumped to previous cue point at {prev.time} (count={self._rwd_cue_count})")
      else:
        Logger.info("No previous cue point!")
    except Exception as e:
      Logger.error(f"Failed to jump to previous cue point: {e}")

  @fwd_cue_button.pressed
  def fwd_cue_button(self, _):
    import time
    Logger.info("fwd_cue_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      bar = int(current_pos // song.signature_numerator)
      now = time.time()
      # Reset counter if we're on a new bar
      if not hasattr(self, '_fwd_cue_count'):
        self._fwd_cue_count = 1
        self._last_fwd_cue_time = 0
        self._last_fwd_bar_checked = None
      if self._last_fwd_bar_checked is None or bar != self._last_fwd_bar_checked:
        self._fwd_cue_count = 1
      elif now - self._last_fwd_cue_time < 0.5:
        self._fwd_cue_count += 1
      else:
        self._fwd_cue_count = 1
      self._last_fwd_cue_time = now
      self._last_fwd_bar_checked = bar

      cue_points = sorted(song.cue_points, key=lambda c: c.time)
      # Find all cue_points after current position
      nexts = [cue for cue in cue_points if cue.time > current_pos]
      if nexts:
        idx = min(len(nexts) - 1, self._fwd_cue_count - 1)
        next_cue = nexts[idx]
        next_cue.jump()
        Logger.info(f"Jumped to next cue point at {next_cue.time} (count={self._fwd_cue_count})")
      else:
        Logger.info("No next cue point!")
    except Exception as e:
      Logger.error(f"Failed to jump to next cue point: {e}")
      
  @rwd_bar_button.pressed
  def rwd_bar_button(self, _):
    Logger.info("rwd_bar_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      new_pos = max(0.0, current_pos - song.signature_numerator)
      song.current_song_time = new_pos
      Logger.info(f"Jumped back one bar to {new_pos}")
    except Exception as e:
      Logger.error(f"Failed to jump back one bar: {e}")
      
  @fwd_bar_button.pressed
  def fwd_bar_button(self, _):
    Logger.info("fwd_bar_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      new_pos = current_pos + song.signature_numerator
      song.current_song_time = new_pos
      Logger.info(f"Jumped forward one bar to {new_pos}")
    except Exception as e:
      Logger.error(f"Failed to jump forward one bar: {e}")
      
  @rwd_bit_button.pressed
  def rwd_bit_button(self, _):
    Logger.info("rwd_bit_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      new_pos = max(0.0, current_pos - (song.signature_numerator / 4.0))
      song.current_song_time = new_pos
      Logger.info(f"Jumped back one bit to {new_pos}")
    except Exception as e:
      Logger.error(f"Failed to jump back one bit: {e}")
      
  @fwd_bit_button.pressed
  def fwd_bit_button(self, _):
    Logger.info("fwd_bit_button pressed!")
    try:
      song = self.song
      current_pos = song.current_song_time
      new_pos = current_pos + (song.signature_numerator / 4.0)
      song.current_song_time = new_pos
      Logger.info(f"Jumped forward one bit to {new_pos}")
    except Exception as e:
      Logger.error(f"Failed to jump forward one bit: {e}")