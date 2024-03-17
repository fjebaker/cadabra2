
#include "TeXView.hh"
#include <iostream>
#include <cairo/cairo.h>
#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <regex>

#ifdef USE_MICROTEX
  #include "platform/cairo/graphic_cairo.h"
#endif

using namespace cadabra;

TeXView::TeXView(TeXEngine& eng, DTree::iterator it, int hmargin)
	: content(0), datacell(it), vbox(false, 10), hbox(false, hmargin), engine(eng)
	{
	// Still need to checkin even when using MicroTeX, otherwise
	// all requests will be empty.
	content = engine.checkin(datacell->textbuf, "", "");
	
#if GTKMM_MINOR_VERSION>=10
	add(rbox);
	rbox.add(vbox);
	rbox.set_reveal_child(false);
	rbox.set_transition_duration(1); // 000);
	rbox.set_transition_type(Gtk::REVEALER_TRANSITION_TYPE_CROSSFADE); //SLIDE_DOWN);
#else
	add(vbox);
#endif
	vbox.set_margin_top(10);
	vbox.set_margin_bottom(0);
	vbox.pack_start(hbox, true, 0);
	hbox.pack_start(image, true, hmargin);
	//	 add(image);
	add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
	}

TeXView::~TeXView()
	{
	engine.checkout(content);
	}

void TeXView::on_show()
	{
	// std::cerr << "**** show called" << std::endl;
	convert();
	Gtk::EventBox::on_show();
	}

Gtk::SizeRequestMode TeXView::TeXArea::get_request_mode_vfunc() const
	{
	return Gtk::SizeRequestMode::SIZE_REQUEST_HEIGHT_FOR_WIDTH;
	}

void TeXView::TeXArea::get_preferred_height_for_width_vfunc(int width,
																				int& minimum_height, int& natural_height) const
	{
#ifdef USE_MICROTEX
//	Gtk::Widget::get_preferred_height_for_width_vfunc(width, minimum_height, natural_height);
//	return;
	
	if(width==1) {
		minimum_height=9999;
		natural_height=9999;
		// std::cerr << "**** skipped bogus width" << std::endl;
		}
	else {
		int remember = rendering_width;
		rendering_width = width;
		layout_latex();
		int extra = (int) (_padding * 2);
		minimum_height = _render->getHeight() + extra;
		natural_height = _render->getHeight() + extra;
		std::cerr << "**** computed for width " << width << " height as " << natural_height << std::endl;
		if(rendering_width==9999)
			rendering_width = remember;
		}
	std::cerr << "**** asked height for width " << width << ", replied " << minimum_height << std::endl;
#else
	Gtk::Widget::get_preferred_height_for_width_vfunc(width, minimum_height, natural_height);
#endif
	}

void TeXView::TeXArea::get_preferred_width_for_height_vfunc(int height,
																				int& minimum_width, int& natural_width) const
	{
	Gtk::Widget::get_preferred_width_for_height_vfunc(height, minimum_width, natural_width);
	return;
	
	if(height==0) {
		minimum_width = 99999;
		natural_width = 99999;
		}
	else {
		minimum_width = rendering_width;
		natural_width = rendering_width;
		}
	std::cerr << "**** asked width for height " << height << ", replied " << minimum_width << std::endl;
	}

void TeXView::TeXArea::on_size_allocate(Gtk::Allocation& allocation)
	{
	set_allocation(allocation);
	std::cerr << "**** offered allocation " << allocation.get_width()
				 << " x " << allocation.get_height() << std::endl;
	if(allocation.get_width() != rendering_width) {
		std::cerr << "**** need to rerender" << std::endl;
		rendering_width = allocation.get_width();
		layout_latex();
		}
	int extra = (int) (_padding * 2);
	int my_height = _render->getHeight() + extra;
	if(allocation.get_height() != my_height) {
		std::cerr << "*** need new height " << my_height << std::endl;
//		set_size_request(rendering_width, my_height);
//		queue_resize();
//		queue_draw();
		}
	}

void TeXView::convert()
	{
//	std::cerr << "*** convert called" << std::endl;
	try {
		// Ensure that all TeX cells have been rendered by TeX. This will do nothing
		// if no TeX cells need (re-)rendering. When adding many cells in one go, do so
		// in hidden state. Then, at first show, the first cell will trigger the
		// convert_all and run TeX on all cells in one shot.

		engine.convert_all();

		// Set the Pixbuf to the image generated by engine.
		// The `content` variable is the TeXRequest.
		image.update_image(content, engine.get_scale());
		}
	catch(TeXEngine::TeXException& ex) {
		tex_error.emit(ex.what());
		}

#ifndef USE_MICROTEX
	image.update_image(content, engine.get_scale());
#endif
	}


void TeXView::dim(bool d)
	{
	if(d) image.set_opacity(0.3);
	else  image.set_opacity(1.0);
	}

bool TeXView::on_button_release_event(GdkEventButton *)
	{
	show_hide_requested.emit(datacell);
	return true;
	}

void TeXView::update_image()
	{
	image.update_image(content, engine.get_scale());
	}

void TeXView::TeXArea::update_image(std::shared_ptr<TeXEngine::TeXRequest> content, double scale)
	{
#ifdef USE_MICROTEX
	if(content) {
		if(content->latex()!=unfixed)
			set_latex(content->latex());
		}
#else
	if(content->image().size()==0)
		return;
	
	if(content->image().data()==0)
		return;
	

	pixbuf =
	   Gdk::Pixbuf::create_from_data(content->image().data(), Gdk::COLORSPACE_RGB,
	                                 true,
	                                 8,
	                                 content->width(), content->height(),
	                                 4*content->width());

	if(pixbuf)
		set_size_request(pixbuf->get_width(), pixbuf->get_height());

	//	update=true;
	scale_=scale;
	// HERE
	//	image.set(pixbuf);
#endif
	}

#ifdef USE_MICROTEX
void TeXView::TeXArea::layout_latex() const
	{
	if(_render)
		delete _render;

	_render = tex::LaTeX::parse(
      tex::utf82wide(fixed),
		rendering_width,
		//  get_allocated_width() - _padding * 2,
      _text_size,
      _text_size / 3.f,
      0xff424242);
	}
#endif

bool TeXView::TeXArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
	{
#ifdef USE_MICROTEX

	// FIXME: this gets called *many* times, not clear why.
	
//	std::cerr << "*** blitting at size " << get_width() << " x " << get_height() << std::endl;

	cr->set_source_rgb(1, 1, 1);
	cr->rectangle(0, 0, get_width(), get_height());
	cr->fill();
	if (_render == nullptr) return true;
	tex::Graphics2D_cairo g2(cr);
	_render->draw(g2, _padding, _padding);
	return true;

#else

	if(!pixbuf) return false;

	//	Gtk::Allocation allocation = get_allocation();
	//	const int width = allocation.get_width();
	//	const int height = allocation.get_height();
	auto surface = cr->get_target();
	auto csurface = surface->cobj();
	//	cairo_surface_set_device_scale(csurface, 1.0, 1.0);
	//	cairo_surface_mark_dirty(csurface);
	double device_scale_x, device_scale_y;
	cairo_surface_get_device_scale(csurface, &device_scale_x, &device_scale_y);
	//	std::cerr << device_scale_x << std::endl;
	set_size_request(pixbuf->get_width()/device_scale_x, pixbuf->get_height()/device_scale_y+1);
	cr->scale(1.0/device_scale_x, 1.0/device_scale_y);
	Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
	cr->paint();
	cr->scale(1.0, 1.0);

	return true;
#endif
	}

#ifdef USE_MICROTEX
void TeXView::TeXArea::check_invalidate()
	{
	std::cerr << "**** check invalidate" << std::endl;

	if (_render == nullptr) return;
	
	int parent_width = get_parent()->get_width();
	int parent_height = get_parent()->get_height();
	int target_width = parent_width;
	int target_height = parent_height;
	
	int extra = (int) (_padding * 2);
	if (parent_width < _render->getWidth() + extra) {
      target_width = _render->getWidth() + extra;
		}
	if (parent_height < _render->getHeight() + extra) {
      target_height = _render->getHeight() + extra;
		}

//	std::cerr << "**** adjust size request " << target_width << " x " << target_height <<
//		" in parent " << parent_width << " x " << parent_height << std::endl;
//	set_size_request(target_width, target_height);
	queue_resize();
	
//	auto win = get_window();
//	if (win) {
//		std::cerr << "**** invalidating rect" << std::endl;
//      auto al = get_allocation();
//      Gdk::Rectangle r(0, 0, al.get_width(), al.get_height());
//      win->invalidate_rect(r, false);
//		queue_draw();
//		}
	}

void TeXView::TeXArea::set_latex(const std::string& latex)
	{
	// std::cout << "**** fixing latex " << latex << std::endl;

	unfixed = latex;
	if(latex.find(R"(\begin{dmath*})")==0) {
		// math mode
		std::regex begin_dmath(R"(\\begin\{dmath\*\})");
		std::regex end_dmath(R"(\\end\{dmath\*\})");
		std::regex discretionary(R"(\\discretionary)");
		std::regex spacenewline(R"(\\\\\[.*\])");
		fixed = std::regex_replace(latex, begin_dmath, "");
		fixed = std::regex_replace(fixed, end_dmath, "");
		fixed = std::regex_replace(fixed, discretionary, "");
		fixed = std::regex_replace(fixed, spacenewline, "\\\\");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\+)"),
											" + ");
		}
	else {
		// text mode

		fixed = std::regex_replace(latex,
											std::regex("\n"),
											" ");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\section\*\{([^\}]*)\}\s*)"),
											"\\text{\\Large\\bf{}$1}\\\\\n\\vspace{1.5ex}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\subsection\*\{([^\}]*)\}\s*)"),
											"\\text{\\large\\bf{}$1}\\\\\n\\vspace{1ex}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\algo\{(.*)\})"),
											"{\\tt{}$1}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\prop\{(.*)\})"),
											"{\\tt{}$1}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\verb\|([^\|]*)\|)"),
											"{\\tt{}$1}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\begin\{verbatim\})"),
											"{\\tt{}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\end\{verbatim\})"),
											"}");
		fixed = "\\text{"+fixed+"}";
		}

	std::cout << "**** fixed to " << fixed << std::endl;
	}

#endif

TeXView::TeXArea::TeXArea()
	: rendering_width(1)
#ifdef USE_MICROTEX
	, _render(nullptr), _text_size(30.f), _padding(10)
#endif
	{
	set_hexpand(true);
	}

TeXView::TeXArea::~TeXArea()
	{
#ifdef USE_MICROTEX
	if(_render)
		delete _render;
#endif
	}

