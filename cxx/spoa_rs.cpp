#include "spoa_rs/cxx/spoa_rs.hpp"
#include <algorithm>
#include <sstream> 

namespace spoa_rs {
    using std::int8_t;
    using std::unique_ptr;
    using ::spoa::AlignmentType;

    unique_ptr<spoa::Graph> new_graph() {
        return std::make_unique<spoa::Graph>();
    }

    size_t graph_node_count(unique_ptr<spoa::Graph> const& graph) {
        return graph->nodes().size();
    }

    size_t graph_edge_count(unique_ptr<spoa::Graph> const& graph) {
        return graph->edges().size();
    }

    unique_ptr<std::string> generate_consensus(unique_ptr<spoa::Graph> const& graph) {
        return std::make_unique<std::string>(graph->GenerateConsensus());
    }

    unique_ptr<std::vector<std::string>> generate_msa(unique_ptr<spoa::Graph> const& graph) {
        return std::make_unique<std::vector<std::string>>(graph->GenerateMultipleSequenceAlignment());
    }

    unique_ptr<spoa::AlignmentEngine> create_alignment_engine_linear(spoa::AlignmentType type, int8_t score_match, int8_t score_mismatch,
                                                                     int8_t score_gap) {
       return spoa::AlignmentEngine::Create(type, score_match, score_mismatch, score_gap);
    }

    unique_ptr<spoa::AlignmentEngine> create_alignment_engine_affine(spoa::AlignmentType type, int8_t score_match, int8_t score_mismatch,
                                                                     int8_t score_gap_open, int8_t score_gap_extend) {
        return spoa::AlignmentEngine::Create(type, score_match, score_mismatch, score_gap_open, score_gap_extend);
    }

    unique_ptr<spoa::AlignmentEngine> create_alignment_engine_convex(spoa::AlignmentType type, int8_t score_match, int8_t score_mismatch,
                                                                     int8_t score_gap_open, int8_t score_gap_extend,
                                                                     int8_t score_gap_open2, int8_t score_gap_extend2) {
        return spoa::AlignmentEngine::Create(type, score_match, score_mismatch, score_gap_open, score_gap_extend,
                                             score_gap_open2, score_gap_extend2);
    }

    unique_ptr<spoa::Alignment> align(unique_ptr<spoa::AlignmentEngine>& engine, rust::Str sequence, unique_ptr<spoa::Graph> const& graph, std::int32_t& score) {
        return std::make_unique<spoa::Alignment>(engine->Align(sequence.data(), sequence.length(), *graph, &score));
    }

    void add_alignment(unique_ptr<spoa::Graph>& graph, unique_ptr<spoa::Alignment> const& alignment, rust::Str sequence) {
        graph->AddAlignment(*alignment, sequence.data(), sequence.length());
    }

    void add_alignment_with_weights(unique_ptr<spoa::Graph>& graph, unique_ptr<spoa::Alignment> const& alignment, rust::Str sequence, rust::Slice<const uint32_t> weights) {
        auto cpp_weights = std::vector<uint32_t>(weights.length());
        std::copy(weights.begin(), weights.end(), cpp_weights.begin());

        graph->AddAlignment(*alignment, sequence.data(), sequence.length(), cpp_weights);
    }

    std::unique_ptr<std::string> generate_gfa(
        const std::unique_ptr<spoa::Graph>& graph,
        rust::Slice<const rust::String> headers,
        bool include_consensus) {
        
        // Convert rust::Slice<const rust::String> to std::vector<std::string>
        std::vector<std::string> cpp_headers;
        cpp_headers.reserve(headers.size());
        for (const auto& header : headers) {
            cpp_headers.push_back(std::string(header));
        }
        
        if (cpp_headers.size() < graph->sequences().size()) {
            throw std::runtime_error("Missing header(s) for GFA generation");
        }

        std::ostringstream oss;
        
        // Generate consensus nodes for marking
        graph->GenerateConsensus();
        std::vector<bool> is_consensus_node(graph->nodes().size(), false);
        for (const auto& it : graph->consensus()) {
            is_consensus_node[it->id] = true;
        }

        // Write header
        oss << "H\tVN:Z:1.0\n";
        
        // Write segments (nodes)
        for (const auto& it : graph->nodes()) {
            oss << "S\t" 
                << it->id + 1 << "\t" 
                << static_cast<char>(graph->decoder(it->code));
            if (is_consensus_node[it->id]) {
                oss << "\tic:Z:true";
            }
            oss << "\n";
        }
        
        // Write links (edges)
        for (const auto& it : graph->nodes()) {
            for (const auto& jt : it->outedges) {
                oss << "L\t"
                    << it->id + 1 << "\t"
                    << "+\t"
                    << jt->head->id + 1 << "\t"
                    << "+\t"
                    << "0M\t"
                    << "ew:f:" << jt->weight;
                if (is_consensus_node[it->id] && is_consensus_node[jt->head->id]) {
                    oss << "\tic:Z:true";
                }
                oss << "\n";
            }
        }
        
        // Write paths (sequences)
        for (std::uint32_t i = 0; i < graph->sequences().size(); ++i) {
            oss << "P\t" << cpp_headers[i] << "\t";
            
            std::vector<std::uint32_t> path;
            auto curr = graph->sequences()[i];
            while (curr) {
                path.emplace_back(curr->id + 1);
                curr = curr->Successor(i);
            }
            
            for (std::uint32_t j = 0; j < path.size(); ++j) {
                if (j != 0) {
                    oss << ",";
                }
                oss << path[j] << "+";
            }
            oss << "\t*\n";
        }
        
        // Write consensus path if requested
        if (include_consensus) {
            oss << "P\tConsensus\t";
            for (std::uint32_t i = 0; i < graph->consensus().size(); ++i) {
                if (i != 0) {
                    oss << ",";
                }
                oss << graph->consensus()[i]->id + 1 << "+";
            }
            oss << "\t*\n";
        }
        
        return std::make_unique<std::string>(oss.str());
    }
}