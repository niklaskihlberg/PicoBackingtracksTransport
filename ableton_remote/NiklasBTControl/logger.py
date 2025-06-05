import logging
import os

Logger = logging.getLogger("niklas_bt_control_logger")

# --- Setup file logging -- #
module_path = os.path.dirname(os.path.realpath(__file__))
log_path = os.path.join(module_path, "NiklasBTControl.log")
file_handler = logging.FileHandler(log_path, mode="a")
file_handler.setLevel(logging.INFO)
formatter = logging.Formatter("(%(asctime)s) [%(levelname)s] %(message)s")
file_handler.setFormatter(formatter)
if not any(isinstance(h, logging.FileHandler) and getattr(h, 'baseFilename', None) == file_handler.baseFilename for h in Logger.handlers):
  Logger.addHandler(file_handler)
Logger.setLevel(logging.INFO)

