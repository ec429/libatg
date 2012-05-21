atg - a tiny gui

Specification

atg is a small, simple GUI library/toolkit for SDL, providing things like buttons and clickables to allow you to concentrate on your program logic.

atg is loosely based on Spiffy's GUI, though genericised.

atg Event Queue:
	atg will poll SDL for events, and if these events trigger any atg elements, these elements' events will be placed on the queue.  In any case all SDL events will be placed on the atg queue, in case the application wishes to further process them.  Note that if the application calls SDL_PollEvent() or SDL_WaitEvent(), these events will not be 'seen' by atg.  Thus, the application should instead call atg_poll_event() or atg_wait_event().  Alternatively, after calling SDL_PollEvent() or SDL_WaitEvent(), the application can pass the returned event to atg with atg_do_event().
