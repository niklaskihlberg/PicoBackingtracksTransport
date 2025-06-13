from ableton.v3.control_surface import Component
from ableton.v3.control_surface.components import TransportComponent
from ableton.v3.control_surface.controls import ButtonControl
from .logger import Logger

class Transport(TransportComponent):
  pass
  
# --- LoopComponent with ButtonControl --- #
class Buttons(Component):
  
  # follow_button  = ButtonControl()
  rwd_bar_button = ButtonControl()
  fwd_bar_button = ButtonControl()
  rwd_bit_button = ButtonControl()
  fwd_bit_button = ButtonControl()



  def __init__(self, *a, **k):
    super().__init__(*a, **k)
    self._control_surface = None  # Will be set after instantiation



  # @follow_button.pressed
  # def follow_button(self, button):
  #   if getattr(button, "value", 127) == 127:
  #     try: self.song.follow_song = False if self.song.loop else True
  #     except Exception as e: Logger.error(f"{e}")



  @rwd_bar_button.pressed
  def rwd_bar_button(self, _):
    try:
      ancora = max(0.0, self.song.current_song_time - self.song.signature_numerator) # Hoppa bakåt en bar, men aldrig före start
      ancora = (ancora - (ancora % self.song.signature_numerator)) + (self.song.current_song_time % self.song.signature_numerator) # Behåll samma position i takten
      self.song.current_song_time = max(0.0, ancora) # Sätt den nya tidpositionen
      Logger.info(f"Hoppade till föregående takt: {ancora}")
    except Exception as e: Logger.error(f"Failed to rewind one bar: {e}")



  @fwd_bar_button.pressed
  def fwd_bar_button(self, _):
    try:
      avanti = self.song.current_song_time + self.song.signature_numerator # Hoppa fram en bar
      avanti = (avanti - (avanti % self.song.signature_numerator)) + (self.song.current_song_time % self.song.signature_numerator) # Behåll samma position i takten
      self.song.current_song_time = avanti # Sätt den nya tidpositionen
      Logger.info(f"Hoppade till nästa takt: {avanti}")
    except Exception as e: Logger.error(f"Failed to skip one bar: {e}")



  @rwd_bit_button.pressed
  def rwd_bit_button(self, _):
    try:
      _live_time = self.song.current_song_time
      self.song.current_song_time = max(0.0, _live_time - (_live_time / 4.0))
      Logger.info(f"Jumped back one beat")
    except Exception as e: Logger.error(f"Failed to jump back one bit: {e}")



  @fwd_bit_button.pressed
  def fwd_bit_button(self, _):
    try:
      _live_time = self.song.current_song_time
      self.song.current_song_time = _live_time + (_live_time / 4.0)
      Logger.info(f"Skipped a beat")
    except Exception as e: Logger.error(f"Failed to jump forward one bit: {e}")


