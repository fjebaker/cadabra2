
#include "Evaluator.hh"
#include "Compare.hh"
#include <cmath>

using namespace cadabra;

void Evaluator::find_common_subexpressions(std::vector<Ex *>)
	{
	// Compute the hash value of every subtree, and collect matches.
	// Then compare subtrees with equal hash to find common subtrees.
	}

double Evaluator::evaluate(const Ex& ex)
	{
	const auto n_sin  = name_set.find("\\sin");
	const auto n_cos  = name_set.find("\\cos");
	const auto n_pow  = name_set.find("\\pow");
	const auto n_prod = name_set.find("\\prod");
	const auto n_sum  = name_set.find("\\sum");

	double lastval=0;

	auto it = ex.begin_post();
	while(it != ex.end_post()) {
		// Either this node is known in the subtree value map,
		// or this node is a function which combines the values
		// of child nodes.


		auto fnd = subtree_values.find(Ex::iterator(it));
		if(fnd!=subtree_values.end()) {
			std::cerr << it << " has value " << fnd->second << std::endl;
			}
		else {
			if(it->name==n_sin) {
				auto arg    = ex.begin(it);
				auto argval = subtree_values.find(arg)->second;
				lastval = std::sin( argval );
				}
			else if(it->name==n_cos) {
				auto arg    = ex.begin(it);
				auto argval = subtree_values.find(arg)->second;
				lastval = std::cos( argval );
				}
			else if(it->name==n_prod) {
				lastval = 1.0;
				for(auto cit = ex.begin(it); cit!=ex.end(it); ++cit) {
					auto cfnd = subtree_values.find(Ex::iterator(cit));
					if(cfnd==subtree_values.end())
						throw std::logic_error("Inconsistent value tree.");
					lastval *= cfnd->second;
					}
				}
			else if(it->name==n_sum) {
				lastval = 0.0;
				for(auto cit = ex.begin(it); cit!=ex.end(it); ++cit) {
					auto cfnd = subtree_values.find(Ex::iterator(cit));
					if(cfnd==subtree_values.end())
						throw std::logic_error("Inconsistent value tree.");
					lastval += cfnd->second;
					}
				}
			else if(it->name==n_pow) {
				std::cerr << "cannot find " << *it << std::endl;
				throw std::logic_error("Value unknown for subtree special function.");
				}
			else {
				// Try variable substitution rules.
				bool found=false;
				for(const auto& var: expression_values) {
					// std::cerr << "Comparing " << var.first << " with " << *it << std::endl;
					if(var.first == *it) {
						subtree_values.insert(std::make_pair(it, var.second));
						lastval = var.second;
						// std::cerr << "We know the value of " << *it << std::endl;
						found=true;
						break;
						}
					}
				if(!found)
					throw std::logic_error("Value unknown for subtree.");
				}
			subtree_values.insert(std::make_pair(it, lastval));
			}

		++it;
		}

	return lastval;
	}

void Evaluator::set_variable(const Ex& var, double val)
	{
	expression_values.insert(std::make_pair(var, val));
	}
