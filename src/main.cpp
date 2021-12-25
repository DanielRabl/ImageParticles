#include <qpl/qpl.hpp>

struct update_state {
	qpl::vector2i mouse_position;
	qpl::f64 frame_time;
	qpl::f64 vel_div = 10.0;
	qpl::f64 speed = 4.0;
};

struct particle {
	qpl::vector2f velocity;
	qpl::vector2f position;
	qsf::rgb color;

	void update(update_tate state) {
		auto diff = state.mouse_position - this->position;

		auto length = std::sqrt((diff.x * diff.x) + (diff.y * diff.y));
		qpl::vector2f normal = qpl::vec(0, 0);
		if (length != 0) {
			normal = diff / length;
		}

		this->velocity *= (1 - state.frame_time / state.vel_div);
		this->velocity += normal;
		this->position += this->velocity * state.frame_time * state.speed;
	}
};

struct main_state : qsf::base_state {

	void init() override {
		auto image = this->get_texture("DDRace").copyToImage();
		
		std::vector<qpl::vector2u> pixels;
		for (qpl::u32 y = 0u; y < image.getSize().y; ++y) {
			for (qpl::u32 x = 0u; x < image.getSize().x; ++x) {
				auto c = image.getPixel(x, y);
				auto sum = c.r + c.g + c.b;
				if (sum < (250 * 3)) {
					pixels.push_back(qpl::vector2f(x, y));
				}
			}
		}

		auto particle_size = 500'000;

		auto vel_mul = 2.0;
		auto speed_mul = 2.0;
		this->vel_gen.set_random_range(5.0 * vel_mul, 30.0 * vel_mul);
		this->speed_gen.set_random_range(2.0 * speed_mul, 4.0 * speed_mul);

		this->va.resize(particle_size);
		this->particles.resize(particle_size);
		this->va.set_primitive_type(qsf::primitive_type::points);

		for (auto& i : this->particles) {
			auto angle = qpl::random(qpl::pi * 2);
			auto x = std::cos(angle);
			auto y = std::sin(angle);

			auto element = qpl::random_element(pixels);
			i.position = element;
			i.color = image.getPixel(element.x, element.y);
			auto diff = this->center() - i.position;
			i.velocity = qpl::random(qpl::vector2f(-1, -1), qpl::vector2f(1, 1));
		}
	}
	void updating() override {
		auto f = this->frame_time().secs_f();
		this->vel_gen.update(f);
		this->speed_gen.update(f);

		update_state state;
		state.frame_time = f;
		state.mouse_position = this->event->mouse_position();
		state.vel_div = this->vel_gen.get();
		state.speed = this->speed_gen.get();

		for (auto& i : this->particles) {
			i.update(state);
		}
		for (qpl::u32 i = 0u; i < this->particles.size(); ++i) {
			this->va[i].position = this->particles[i].position;
			this->va[i].color = this->particles[i].color;
		}
	}
	void drawing() override {
		this->draw(this->va);
	}

	qsf::vertex_array va;
	std::vector<particle> particles;
	qpl::cubic_generator vel_gen;
	qpl::cubic_generator speed_gen;
};

int main() {
	qsf::framework framework;
	framework.set_title("Particles");
	framework.set_dimension({ 1600, 900u });
	framework.add_texture("DDRace", "resources/DDRace.png");

	framework.add_state<main_state>();
	framework.game_loop();
}