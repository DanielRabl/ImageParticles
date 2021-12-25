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

	void update(update_state state) {
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
					pixels.push_back(qpl::vec(x, y));
				}
			}
		}

		auto particle_size = 500'000;

		this->cubic_velocity_gen.set_random_range(10.0, 60.0);
		this->cubic_speed_gen.set_random_range(4.0, 8.0);

		this->va.resize(particle_size);
		this->particles.resize(particle_size);
		this->va.set_primitive_type(qsf::primitive_type::points);

		for (auto& i : this->particles) {
			auto pos = qpl::random_element(pixels);
			i.position = pos;
			i.color = image.getPixel(pos.x, pos.y);
			i.velocity = qpl::random(qpl::vector2f(-1, -1), qpl::vector2f(1, 1));
		}
	}
	void updating() override {
		auto f = this->frame_time().secs_f();
		this->cubic_velocity_gen.update(f);
		this->cubic_speed_gen.update(f);

		update_state state;
		state.frame_time = f;
		state.mouse_position = this->event->mouse_position();
		state.vel_div = this->cubic_velocity_gen.get();
		state.speed = this->cubic_speed_gen.get();

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
	qpl::cubic_generator cubic_velocity_gen;
	qpl::cubic_generator cubic_speed_gen;
};

int main() {
	qsf::framework framework;
	framework.set_title("Particles");
	framework.set_dimension({ 1600, 900u });
	framework.add_texture("DDRace", "resources/DDRace.png");

	framework.add_state<main_state>();
	framework.game_loop();
}