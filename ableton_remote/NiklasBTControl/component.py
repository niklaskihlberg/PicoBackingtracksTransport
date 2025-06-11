import time
from ableton.v3.control_surface import Component
from ableton.v3.control_surface.components import TransportComponent
from ableton.v3.control_surface.controls import ButtonControl
from .logger import Logger


class Transport(TransportComponent):
  # Exempel på mapping (i din controller definition)
  pass
  

# --- LoopComponent with ButtonControl --- #
class Buttons(Component):
  
  # loop_button    = ButtonControl()
  # loop_query     = ButtonControl()
  # rwd_cue_button = ButtonControl()
  # fwd_cue_button = ButtonControl()
  rwd_bar_button = ButtonControl()
  fwd_bar_button = ButtonControl()
  rwd_bit_button = ButtonControl()
  fwd_bit_button = ButtonControl()

  def __init__(self, *a, **k):
    super().__init__(*a, **k)
    self._control_surface = None  # Will be set after instantiation

  # @loop_button.pressed
  # def loop_button(self, button):
  #   if getattr(button, "value", 127) == 127:
  #     try:
  #       if (self.song.loop):
  #         self.song.loop = False
  #         Logger.info(f"░░░ 1 ░ Loop set: OFF.")
  #       elif (not self.song.loop):
  #         self.song.loop = True
  #         Logger.info(f"░░░ 1 ░ Loop set: ON.")
  #       else:
  #         Logger.info(f"░░░ 1 ░ Loop unchanged.")
  #     except Exception as e:
  #       Logger.error(f"░░░ 1 ░ Failed to toggle loop: {e}")
        
  # @loop_query.pressed
  # def loop_query(self, button):
  #   try:
  #     self._c_instance.send_midi((0xB0, 0x64, 0x7F if self.song.loop else 0x00)) # Send initial MIDI feedback based on loop state
  #     Logger.info(f"Inquiery respons! Loop: {'on' if self.song.loop else 'off'}!")  # Log feedback
  #   except Exception as e:
  #     Logger.error(f"Failed to query loop state: {e}")



  # @rwd_cue_button.pressed
  # def rwd_cue_button(self, _):
    
  #   try:
      
  #     if (self.rwd_count == 0):
  #       self.rwd_count = 1
        
  #     else:
  #       self.rwd_count += 1

  #     self.cue_points = sorted(self.song.cue_points, key=lambda c: c.time)
  #     self.rwd_points = [cue for cue in self.cue_points if cue.time < self.song.current_song_time]
      
  #     if self.rwd_points:
  #       self.rwd_points[max(0, len(self.rwd_points) - self.rwd_count)].jump()
  #       Logger.info(f"Jumped (count={self.rwd_count}) back!")
      
  #     else:
  #       Logger.info("No previous cue point!")
    
  #   except Exception as e:
  #     Logger.error(f"Failed jump to previous cue point: {e}")
      
      

  # @fwd_cue_button.pressed
  # def fwd_cue_button(self, _):
    
  #   try:

  #     bar = int(self.song.current_song_time // self.song.signature_numerator)
  #     now = time.time()

  #     if not hasattr(self, '_fwd_cue_count'):
  #       self._fwd_cue_count = 1
  #       self._last_fwd_cue_time = 0
  #       self._last_fwd_bar_checked = None

  #     if self._last_fwd_bar_checked is None or bar != self._last_fwd_bar_checked:
  #       self._fwd_cue_count = 1
  #     elif now - self._last_fwd_cue_time < 0.5:
  #       self._fwd_cue_count += 1
  #     else:
  #       self._fwd_cue_count = 1

  #     self._last_fwd_cue_time = now
  #     self._last_fwd_bar_checked = bar
      
      
      
      
      
  #     self.next = int(self.song.current_song_time)
      
      

  #     cue_points = sorted(self.song.cue_points, key=lambda c: c.time)
  #     nexts = [cue for cue in cue_points if cue.time > self.song.current_song_time]

  #     if nexts:
  #       idx = min(len(nexts) - 1, self._fwd_cue_count - 1)
  #       next_cue = nexts[idx]
  #       next_cue.jump()
  #       Logger.info(f"Jumped to next cue point at {next_cue.time} (count={self._fwd_cue_count})")
  #     else:
  #       Logger.info("No next cue point!")

  #   except Exception as e:
  #     Logger.error(f"Failed jump to next cue point: {e}")



  @rwd_bar_button.pressed
  def rwd_bar_button(self, _):
    
    try:
      
      # Hoppa bakåt en bar, men aldrig före start
      ancora = max(0.0, self.song.current_song_time - self.song.signature_numerator)
      
      # Behåll samma position i takten
      ancora = (ancora - (ancora % self.song.signature_numerator)) + (self.song.current_song_time % self.song.signature_numerator)
      
      # Se till att vi inte hamnar före 0
      ancora = max(0.0, ancora)
      
      self.song.current_song_time = ancora
      Logger.info(f"Hoppade till föregående takt: {ancora}")
    
    except Exception as e:
      Logger.error(f"Failed to rewind one bar: {e}")

    
      
  @fwd_bar_button.pressed
  def fwd_bar_button(self, _):
    
    try:

      # Hoppa fram en bar
      avanti = self.song.current_song_time + self.song.signature_numerator
      
      # Behåll samma position i takten
      avanti = (avanti - (avanti % self.song.signature_numerator)) + (self.song.current_song_time % self.song.signature_numerator)
      
      self.song.current_song_time = avanti
      Logger.info(f"Hoppade till nästa takt: {avanti}")
    
    except Exception as e:
      Logger.error(f"Failed to skip one bar: {e}")
      
    
      
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