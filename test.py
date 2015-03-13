import hotmon, vkdefs


def test_1():
	"""Testing the basic C API.
	"""
	
	import threading
	
	event = threading.Event()
	
	def esc_callback(param):
		print "hello, esc_callback!"
		event.set()
		
	def csf1_callback(param):
		print "hello, csf1_callback!"
		
	# these are important! otherwise they'll get garbage collected!
	esc_callback_ref = hotmon.CALLBACK(esc_callback)
	csf1_callback_ref = hotmon.CALLBACK(csf1_callback)
	
	hm = hotmon.PHOTMON()
	
	hotmon.hmCreate(hm)
	hotmon.hmStart(hm)
	
	try:
		quit_hk = hotmon.HOTKEY()
		hotmon.hmCreateHotkey(hm, quit_hk, vkdefs.VK_ESCAPE, 0, esc_callback_ref)
		
		action_hk = hotmon.HOTKEY()
		hotmon.hmCreateHotkey(hm, action_hk, vkdefs.VK_F1, vkdefs.MOD_CONTROL|vkdefs.MOD_SHIFT, csf1_callback_ref)
		
		hotmon.hmAddHotkey(hm, quit_hk)
		hotmon.hmAddHotkey(hm, action_hk)
		
		print "(to quit the app, hit the ESC key)"
		print "(try pressing CTRL+SHIFT+F1)"
		event.wait()
		
	finally:
		hotmon.hmStop(hm)
		hotmon.hmDelete(hm)
		
		
def test_2():
	"""Testing the Hotmon convenience class.
	"""
	
	def esc_callback(param):
		print "hello, esc_callback!"
		hm.stop()
		
	def csf1_callback(param):
		print "hello, csf1_callback!"
	
	hm = hotmon.Hotmon()
	try:
		hm.create()
		hm.add(esc_callback, vkdefs.VK_ESCAPE)
		hm.add(csf1_callback, vkdefs.VK_F1, vkdefs.MOD_CONTROL|vkdefs.MOD_SHIFT)
		
		print "(to quit the app, hit the ESC key)"
		print "(try pressing CTRL+SHIFT+F1)"
		
		hm.wait()
	finally:
		hm.destroy()
		
		
def test_3():
	"""Testing the context manager feature.
	"""
	
	def esc_callback(param):
		print "hello, esc_callback!"
		hm.stop()
		
	def csf1_callback(param):
		print "hello, csf1_callback!"
	
	with hotmon.Hotmon() as hm:
		hm.add(esc_callback, vkdefs.VK_ESCAPE)
		hm.add(csf1_callback, vkdefs.VK_F1, vkdefs.MOD_CONTROL|vkdefs.MOD_SHIFT)
		
		print "(to quit the app, hit the ESC key)"
		print "(try pressing CTRL+SHIFT+F1)"
		
		hm.wait()
		
		
if "__main__" == __name__:
	#test_1()
	#test_2()
	test_3()
