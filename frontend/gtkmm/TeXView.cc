
#include "TeXView.hh"
#include <iostream>
#include <cairo/cairo.h>
#include <cairomm/context.h>
#include <giomm/resource.h>
#include <gdkmm/general.h> // set_source_pixbuf()
#include <regex>

// MicroTeX
#include "cairo/graphic_cairo.h"

using namespace cadabra;

TeXView::TeXView(TeXEngine& eng, DTree::iterator it, bool use_microtex_, int hmargin)
	: content(0), datacell(it), vbox(false, 10), hbox(false, hmargin), image(use_microtex_), engine(eng), use_microtex(use_microtex_)
	{
	// Still need to checkin even when using MicroTeX, otherwise
	// all requests will be empty.
	content = engine.checkin(datacell->textbuf, "", "");
	
// 	add(rbox);
// 	rbox.add(vbox);
// 	rbox.set_reveal_child(false);
// 	rbox.set_transition_duration(1000);
// 	rbox.set_transition_type(Gtk::REVEALER_TRANSITION_TYPE_CROSSFADE); //SLIDE_DOWN);

	vbox.set_margin_top(10);
	vbox.set_margin_bottom(0);
//	vbox.pack_start(hbox, true, 0);
//	image._text_size = text_size();
//	hbox.pack_start(image, true, hmargin);


//	vbox.pack_start(hbox, true, 0);
	image._text_size = text_size();
	add(image);

	add_events( Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK );
	}

TeXView::~TeXView()
	{
	engine.checkout(content);
	}

void TeXView::set_use_microtex(bool use_microtex_)
	{
	use_microtex = use_microtex_;
//	remove(); // 'image' is the only child.
	image.use_microtex = use_microtex_;
	queue_draw();
//	add(image);
	}

float TeXView::text_size() const
	{
	// The device pixel ratio scale is applied in the ::on_draw method,
	// so we do not scale the font size by that here. But we do need
	// to scale by the text scaling factor of the system, which is
	// available in engine.get_scale().
	
	float ret = 28.0f/12.0f*engine.get_font_size()*engine.get_scale()/1.7f;
//	std::cerr << "engine.font_size = " << engine.get_font_size() << ", .scale = " << engine.get_scale()
//				 << ", text_size = " << ret << std::endl;
	return ret;
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
	if(use_microtex) {
		if(width==1) {
			minimum_height=9999;
			natural_height=9999;
			// std::cerr << "**** skipped bogus width" << std::endl;
			}
		else {
			int remember = rendering_width;
			rendering_width = width - 2*padding_x;
			layout_latex();
			minimum_height = _render->getHeight() + 2*padding_y;
			natural_height = _render->getHeight() + 2*padding_y;
         //		std::cerr << "**** computed for width " << width << " height as " << natural_height << std::endl;
			if(rendering_width==9999)
				rendering_width = remember;
			}
      //	std::cerr << "**** asked height for width " << width << ", replied " << minimum_height << std::endl;
		}
	else {
		Gtk::Widget::get_preferred_height_for_width_vfunc(width, minimum_height, natural_height);
		}
	}

void TeXView::TeXArea::get_preferred_width_for_height_vfunc(int height,
																				int& minimum_width, int& natural_width) const
	{
	Gtk::Widget::get_preferred_width_for_height_vfunc(height, minimum_width, natural_width);
	return;
	
//	if(height==0) {
//		minimum_width = 99999;
//		natural_width = 99999;
//		}
//	else {
//		minimum_width = rendering_width + 2 * padding_x;
//		natural_width = rendering_width + 2 * padding_x;
//		}
//	std::cerr << "**** asked width for height " << height << ", replied " << minimum_width << std::endl;
	}

void TeXView::TeXArea::on_size_allocate(Gtk::Allocation& allocation)
	{
	set_allocation(allocation);
//	std::cerr << "**** offered allocation " << allocation.get_width()
//				 << " x " << allocation.get_height() << std::endl;

	if(use_microtex) {
		if(allocation.get_width() != rendering_width + 2*padding_x) {
//		std::cerr << "**** need to rerender" << std::endl;
			rendering_width = allocation.get_width() - 2*padding_x;
			layout_latex();
			}
// 	int extra = (int) (_padding * 2);
// 	int my_height = _render->getHeight() + extra;
// 	if(allocation.get_height() != my_height) {
// //		std::cerr << "*** need new height " << my_height << std::endl;
// //		set_size_request(rendering_width, my_height);
// //		queue_resize();
// //		queue_draw();
// 		}
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

		if(!use_microtex) {
			engine.convert_all();
			}

		// Set the Pixbuf to the image generated by engine.
		// The `content` variable is the TeXRequest.
		image.update_image(content, engine.get_scale());
		}
	catch(TeXEngine::TeXException& ex) {
		tex_error.emit(ex.what());
		}
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
	float new_size = text_size();
	if(image._text_size != new_size) {
		image._text_size = new_size;
		if(use_microtex)
			image.layout_latex();
		}
	image.update_image(content, engine.get_scale());
	}

void TeXView::TeXArea::update_image(std::shared_ptr<TeXEngine::TeXRequest> content, double scale)
	{
	if(use_microtex) {
		if(content) {
			if(content->latex()!=unfixed)
				set_latex(content->latex());
			}
		}
	else {
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
		}
	}

void TeXView::TeXArea::layout_latex() const
	{
	if(_render)
		delete _render;

//	std::cerr << "running layout with text_size = " << _text_size << std::endl;
	_render = microtex::MicroTeX::parse(
      fixed, //microtex::utf82wide(fixed),
		rendering_width,
      _text_size,
      _text_size / 3.f,
      0xff424242);
	}

bool TeXView::TeXArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
	{
	if(use_microtex) {
      //	std::cerr << "*** blitting at size " << get_width() << " x " << get_height() << std::endl;

		cr->set_source_rgb(1, 1, 1);
		cr->rectangle(0, 0, get_width(), get_height());
		
		auto surface = cr->get_target();
		auto csurface = surface->cobj();
		double device_scale_x, device_scale_y;
		cairo_surface_get_device_scale(csurface, &device_scale_x, &device_scale_y);
      //	std::cerr << "scale = " << device_scale_x << ", height = "<< _render->getHeight() << std::endl;
		cr->scale(1.0, 1.0); // /device_scale_x, 1.0/device_scale_y);

		// Don't fill, that will mess up the cell background colour.
		cr->fill();
		if (_render == nullptr) return true;
		microtex::Graphics2D_cairo g2(cr->cobj());
		_render->draw(g2, padding_x, padding_y);
		
		cr->scale(1.0, 1.0);
		return true;
		}
	else {
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
		set_size_request(pixbuf->get_width()/device_scale_x  + 2*padding_x,
							  pixbuf->get_height()/device_scale_y + 1 + 2*padding_y);
		cr->scale(1.0/device_scale_x, 1.0/device_scale_y);
		cr->translate(padding_x, padding_y);
		Gdk::Cairo::set_source_pixbuf(cr, pixbuf, 0, 0);
		cr->paint();
		cr->scale(1.0, 1.0);
		cr->translate(0, 0);
		
		return true;
		}
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
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\linebreak\[[0-9]\])"),
											"");
		fixed = "\\text{$"+fixed+"$}";
		}
	else {
		// text mode

		fixed = std::regex_replace(latex,
											std::regex("\n"),
											" ");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\section\*\{([^\}]*)\}\s*)"),
											"\\text{\\Large\\bf{}$1}\\\\\n\\vspace{2.5ex}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\subsection\*\{([^\}]*)\}\s*)"),
											"\\text{\\large\\bf{}$1}\\\\\n\\vspace{1.5ex}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\algo\{(.*)\})"),
											"{\\tt{}$1}");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\algorithm\{(.*)\}\{(.*)\})"),
											"\\text{\\large\\tt{}$1}\\\\\\vspace{2.5ex}\\text{\\textit{$2}}\\\\");
		fixed = std::regex_replace(fixed,
											std::regex(R"(\\property\{(.*)\}\{(.*)\})"),
											"\\text{\\large\\tt{}$1}\\\\\\vspace{2.5ex}\\text{\\textit{$2}}\\\\");
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

//	fixed = "\\text{$A_{m n}$}";
//	std::cout << "**** fixed to " << fixed << std::endl;
	}

TeXView::TeXArea::TeXArea(bool use_microtex_)
	: rendering_width(1), use_microtex(use_microtex_)
	, _render(nullptr), _text_size(5.f)
	, padding_x(15), padding_y(20)
	{
	set_hexpand(true);
	}

TeXView::TeXArea::~TeXArea()
	{
	if(_render)
		delete _render;
	}

