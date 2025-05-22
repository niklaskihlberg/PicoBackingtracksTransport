from ableton.v2.control_surface import ControlSurface, MIDI_NOTE_TYPE, MIDI_CC_TYPE, ButtonElement

class NiklasBTT(ControlSurface):
    def __init__(self, c_instance):
        super(NiklasBTT, self).__init__(c_instance)
        self.log_message("Niklas Backing Track Transport script loaded!")

        # CUE/LOCATOR
        self.rwd_cue = ButtonElement(True, MIDI_NOTE_TYPE, 0, 1)
        self.fwd_cue = ButtonElement(True, MIDI_NOTE_TYPE, 0, 4)
        self.rwd_cue.add_value_listener(self.goto_prev_locator)
        self.fwd_cue.add_value_listener(self.goto_next_locator)

        # BAR
        self.rwd_bar = ButtonElement(True, MIDI_NOTE_TYPE, 0, 2)
        self.fwd_bar = ButtonElement(True, MIDI_NOTE_TYPE, 0, 5)
        self.rwd_bar.add_value_listener(self.jump_minus_bar)
        self.fwd_bar.add_value_listener(self.jump_plus_bar)

        # BEAT
        self.rwd_beat = ButtonElement(True, MIDI_NOTE_TYPE, 0, 3)
        self.fwd_beat = ButtonElement(True, MIDI_NOTE_TYPE, 0, 6)
        self.rwd_beat.add_value_listener(self.jump_minus_beat)
        self.fwd_beat.add_value_listener(self.jump_plus_beat)

        # LOOP
        self.loop_button = ButtonElement(True, MIDI_CC_TYPE, 0, 100)
        self.loop_button.add_value_listener(self.toggle_loop)

    def goto_prev_locator(self, value):
        if value > 0:
            song = self.song()
            current_time = song.current_song_time

            # Samla locators
            points = [locator.time for locator in song.locators]

            # Samla clip start/slut på track 1
            track = song.tracks[0]
            for slot in track.clip_slots:
                if slot.has_clip:
                    points.append(slot.clip.start_time)
                    points.append(slot.clip.end_time)

            # Ta bort dubbletter och sortera
            points = sorted(set(points))

            # Hitta närmast föregående punkt
            prev = 0.0
            for t in points:
                if t < current_time:
                    prev = t
                else:
                    break
            song.current_song_time = prev
            self.log_message("Jumped to previous marker/clip: {}".format(prev))

    def goto_next_locator(self, value):
        if value > 0:
            song = self.song()
            current_time = song.current_song_time

            # Samla locators
            points = [locator.time for locator in song.locators]

            # Samla clip start/slut på track 1
            track = song.tracks[0]
            for slot in track.clip_slots:
                if slot.has_clip:
                    points.append(slot.clip.start_time)
                    points.append(slot.clip.end_time)

            # Ta bort dubbletter och sortera
            points = sorted(set(points))

            # Hitta närmast nästa punkt
            next_time = None
            for t in points:
                if t > current_time:
                    next_time = t
                    break
            if next_time is not None:
                song.current_song_time = next_time
                self.log_message("Jumped to next marker/clip: {}".format(next_time))

    def jump_minus_bar(self, value):
        if value > 0:
            song = self.song()
            song.jump_by(-song.signature_numerator)
            self.log_message("Jumped a bar backward")

    def jump_plus_bar(self, value):
        if value > 0:
            song = self.song()
            song.jump_by(song.signature_numerator)
            self.log_message("Jumped a bar forward")

    def jump_minus_beat(self, value):
        if value > 0:
            song = self.song()
            song.jump_by(-1.0)
            self.log_message("Repeated a beat")

    def jump_plus_beat(self, value):
        if value > 0:
            song = self.song()
            song.jump_by(1.0)
            self.log_message("Skipped a beat")

    def toggle_loop(self, value):
        if value > 0:
            song = self.song()
            song.loop = not song.loop
            self.log_message("Loop toggled: {}".format(song.loop))