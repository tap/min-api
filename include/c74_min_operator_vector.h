/// @file
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {


	struct audio_bundle {

		audio_bundle(double** samples, long channelcount, long framecount)
		: m_samples			{ samples }
		, m_channelcount	{ channelcount }
		, m_framecount		{ framecount }
		{}

		double** samples() {
			return m_samples;
		}

		double* samples(size_t channel) {
			return m_samples[channel];
		}

		long channelcount() {
			return m_channelcount;
		}

		long framecount() {
			return m_framecount;
		}

		void clear() {
			for (auto channel=0; channel < m_channelcount; ++channel) {
				for (auto i=0; i < m_framecount; ++i)
					m_samples[channel][i] = 0.0;
			}
		}


		/// Copy an audio_bundle to another audio_bundle without resizing the destination
		/// If the destination does not have enough channels to copy the entire source
		/// the extra channels will be dropped.

		audio_bundle& operator = (const audio_bundle& other) {
			assert(m_channelcount <= other.m_channelcount);
			assert(m_framecount == other.m_framecount);

			for (auto channel=0; channel < m_channelcount; ++channel) {
				for (auto i=0; i < m_framecount; ++i)
					m_samples[channel][i] = other.m_samples[channel][i];
			}
			return *this;
		}

		double**	m_samples = nullptr;
		long		m_channelcount = 0;
		long		m_framecount = 0;
	};
	

	class vector_operator_base {};
	class vector_operator : public vector_operator_base {
	public:

		void samplerate_set(double a_samplerate) {
			m_samplerate = a_samplerate;
		}

		double samplerate() {
			return m_samplerate;
		}

		sample operator()(sample x);

		virtual void operator()(audio_bundle input, audio_bundle output) = 0;


	private:
		double m_samplerate { c74::max::sys_getsr() };
	};
	
	
	template<class min_class_type>
	struct minwrap <min_class_type, type_enable_if_audio_class<min_class_type> > {
		maxobject_base	max_base;
		min_class_type	min_object;
		
		void setup() {
			max::dsp_setup(max_base, (long)min_object.inlets().size());

			max::t_pxobject* x = max_base;
			x->z_misc |= max::Z_NO_INPLACE;
			
			min_object.create_outlets();
		}
		
		void cleanup() {
			max::dsp_free(max_base);
		}

		max::t_object* maxobj() {
			return max_base;
		}
	};
	
	
	

	// the partial specialization of A is enabled via a template parameter
	template<class min_class_type, class Enable = void>
	class min_performer {
	public:
		static void perform(minwrap<min_class_type>* self, max::t_object *dsp64, double **in_chans, long numins, double **out_chans, long numouts, long sampleframes, long flags, void *userparam) {
			audio_bundle input = {in_chans, numins, sampleframes};
			audio_bundle output = {out_chans, numouts, sampleframes};
			self->min_object(input, output);
		}
	}; // primary template
	
	
	template<typename min_class_type>
	struct has_dspsetup {
		template<class,class> class checker;
		
		template<typename C>
		static std::true_type test(checker<C, decltype(&C::dspsetup)> *);
		
		template<typename C>
		static std::false_type test(...);
		
		typedef decltype(test<min_class_type>(nullptr)) type;
		static const bool value = is_same<std::true_type, decltype(test<min_class_type>(nullptr))>::value;
	};
	
	//	static_assert(has_dspsetup<slide>::value, "error");
	
	
	template<class min_class_type>
	void min_dsp64_io(minwrap<min_class_type>* self, short* count) {
		int i = 0;
		
		while (i < self->min_object.inlets().size()) {
			self->min_object.inlets()[i]->update_signal_connection(count[i]!=0);
			++i;
		}
		while (i < self->min_object.outlets().size()) {
			self->min_object.outlets()[i - self->min_object.inlets().size()]->update_signal_connection(count[i]!=0);
			++i;
		}
	}
	
	
	// TODO: enable a different add_perform if no perform with the correct sig is available?
	// And/or overload the min_perform for different input and output counts?
	
	template<class min_class_type>
	void min_dsp64_add_perform(minwrap<min_class_type>* self, max::t_object* dsp64) {
		// find the perform method and add it
		object_method_direct(void, (max::t_object*, max::t_object*, max::t_perfroutine64, long, void*),
							 dsp64, symbol("dsp_add64"), self->maxobj(), reinterpret_cast<max::t_perfroutine64>(min_performer<min_class_type>::perform), 0, NULL);
	}
	
	template<class min_class_type>
	typename enable_if< has_dspsetup<min_class_type>::value && is_base_of<vector_operator_base, min_class_type>::value>::type
	min_dsp64_sel(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		self->min_object.samplerate_set(samplerate);
		min_dsp64_io(self, count);

		atoms args;
		args.push_back(atom(samplerate));
		args.push_back(atom(maxvectorsize));
		self->min_object.dspsetup(args);
		
		min_dsp64_add_perform(self, dsp64);
	}
	
	template<class min_class_type>
	typename enable_if< !has_dspsetup<min_class_type>::value>::type
	min_dsp64_sel(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		self->min_object.samplerate_set(samplerate);
		min_dsp64_io(self, count);
		min_dsp64_add_perform(self, dsp64);
	}
	
	template<class min_class_type>
	type_enable_if_audio_class<min_class_type> min_dsp64(minwrap<min_class_type>* self, max::t_object* dsp64, short* count, double samplerate, long maxvectorsize, long flags) {
		min_dsp64_sel<min_class_type>(self, dsp64, count, samplerate, maxvectorsize, flags);
	}


	template<class min_class_type, enable_if_vector_operator<min_class_type> = 0>
	void wrap_as_max_external_audio(max::t_class* c) {
		max::class_addmethod(c, reinterpret_cast<max::method>(min_dsp64<min_class_type>), "dsp64", max::A_CANT, 0);
		max::class_dspinit(c);
	}
}} // namespace c74::min
