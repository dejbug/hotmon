import threading
import hotmon, vkdefs


if "__main__" == __name__:
	event = threading.Event()
	
	def quit_callback(param):
		event.set()
		
	def action_callback(param):
		print "f1"
		
	# these are important! otherwise they'll get garbage collected!
	quit_callback_ref = hotmon.CALLBACK(quit_callback)
	action_callback_ref = hotmon.CALLBACK(action_callback)
	
	hm = hotmon.PHOTMON()
	hotmon.hmCreate(hm)
	hotmon.hmStart(hm)
	
	quit_hk = hotmon.HOTKEY()
	hotmon.hmCreateHotkey(hm, quit_hk, vkdefs.VK_ESCAPE, 0, quit_callback_ref)
	
	action_hk = hotmon.HOTKEY()
	hotmon.hmCreateHotkey(hm, action_hk, vkdefs.VK_F1, vkdefs.MOD_CONTROL|vkdefs.MOD_SHIFT, action_callback_ref)
	
	hotmon.hmAddHotkey(hm, quit_hk)
	hotmon.hmAddHotkey(hm, action_hk)
	
	print "(to quit the app, hit the ESC key)"
	print "(try pressing CTRL+SHIFT+F1)"
	event.wait()
	
	hotmon.hmStop(hm)
	hotmon.hmDelete(hm)
	
