# Graph Report - .  (2026-06-02)

## Corpus Check
- Corpus is ~31,178 words - fits in a single context window. You may not need a graph.

## Summary
- 142 nodes · 185 edges · 14 communities (8 shown, 6 thin omitted)
- Extraction: 86% EXTRACTED · 14% INFERRED · 1% AMBIGUOUS · INFERRED: 25 edges (avg confidence: 0.89)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [[_COMMUNITY_Microsoft Extensions Packages|Microsoft Extensions Packages]]
- [[_COMMUNITY_TMG1 Core Codec (C)|TMG1 Core Codec (C#)]]
- [[_COMMUNITY_Build & Test Infrastructure|Build & Test Infrastructure]]
- [[_COMMUNITY_Documentation & Format Concepts|Documentation & Format Concepts]]
- [[_COMMUNITY_Arduino Decoder Tests|Arduino Decoder Tests]]
- [[_COMMUNITY_C++ Decoder Implementation|C++ Decoder Implementation]]
- [[_COMMUNITY_TMG1 Encoder & Stream Writer|TMG1 Encoder & Stream Writer]]
- [[_COMMUNITY_.NET Solution Structure|.NET Solution Structure]]
- [[_COMMUNITY_Diagnostics Packages|Diagnostics Packages]]
- [[_COMMUNITY_Arduino Stubs (C++)|Arduino Stubs (C++)]]
- [[_COMMUNITY_NUnit Configuration|NUnit Configuration]]
- [[_COMMUNITY_Arduino HAL Stubs|Arduino HAL Stubs]]
- [[_COMMUNITY_Claude Code Settings|Claude Code Settings]]
- [[_COMMUNITY_Dev Container Config|Dev Container Config]]

## God Nodes (most connected - your core abstractions)
1. `Microsoft.Extensions.Hosting 8.0.0` - 23 edges
2. `Tmg1Decoder` - 11 edges
3. `TMG1 Encoder .NET README (English)` - 11 edges
4. `PayloadCompressor` - 10 edges
5. `Tmg1Decoder (C++ Implementation)` - 10 edges
6. `TMG1 Encoder .NET README (Japanese)` - 10 edges
7. `TMG1 Encoder .NET Project` - 10 edges
8. `Seizu.Tmg1.Core Library` - 10 edges
9. `Test Main Runner` - 9 edges
10. `Seizu.Tmg1.Tests` - 8 edges

## Surprising Connections (you probably didn't know these)
- `test_decode_simple_frames` --references--> `P-Frame Inter-frame Differential Compression`  [EXTRACTED]
  test/test_Tmg1Decoder.cpp → .gemini/tmp/encoder_repo/README.md
- `test_decode_file_from_simple_tmg1` --references--> `P-Frame Inter-frame Differential Compression`  [EXTRACTED]
  test/test_Tmg1Decoder.cpp → .gemini/tmp/encoder_repo/README.md
- `Decoder GitLab CI Pipeline` --references--> `Test Main Runner`  [INFERRED]
  .gitlab-ci.yml → test/test_main.cpp
- `PredictionFilterTests (C# Test Fixture)` --conceptually_related_to--> `Tmg1Decoder (C++ Implementation)`  [INFERRED]
  .gemini/tmp/encoder_repo/tests/Seizu.Tmg1.Tests/PredictionFilterTests.cs → src/Tmg1Decoder.cpp
- `Tmg1DecoderTests (C# Test Fixture)` --conceptually_related_to--> `Tmg1Decoder (C++ Implementation)`  [INFERRED]
  .gemini/tmp/encoder_repo/tests/Seizu.Tmg1.Tests/Tmg1DecoderTests.cs → src/Tmg1Decoder.cpp

## Import Cycles
- None detected.

## Hyperedges (group relationships)
- **Rice Codec Pair (Encoder + Decoder)** — ricecoder_ricecoder, ricebitreader_ricebitreader, ientropywriter_ientropywriter, ientropyreader_ientropyreader [EXTRACTED 1.00]
- **Range Codec Pair (Encoder + Decoder)** — rangeencoder_rangeencoder, rangedecoder_rangedecoder, frequencymodel_frequencymodel, ientropywriter_ientropywriter, ientropyreader_ientropyreader [EXTRACTED 1.00]
- **Frame I/O Interface Pair** — ibitplaneframereader_ibitplaneframereader, ibitplaneframewriter_ibitplaneframewriter, program_rawbitplaneframereader, program_rawbitplaneframewriter [EXTRACTED 1.00]
- **Prediction Filter Pipeline** — predictionfilter_predictionfilter, predictionmethod_predictionmethod, payloadcompressor_payloadcompressor, tmg1decoder_tmg1decoder [EXTRACTED 1.00]
- **C# Encoder Core Components** — Tmg1Encoder_cs_Tmg1Encoder, Tmg1EncoderOptions_cs_Tmg1EncoderOptions, Tmg1StreamWriter_cs_Tmg1StreamWriter [EXTRACTED 0.95]
- **C# Test Fixtures** — FrequencyModelTests_cs_FrequencyModelTests, PayloadCompressorTests_cs_PayloadCompressorTests, PredictionFilterTests_cs_PredictionFilterTests, RangeCoderTests_cs_RangeCoderTests, RiceCoderTests_cs_RiceCoderTests, Tmg1DecoderTests_cs_Tmg1DecoderTests, Tmg1EncoderTests_cs_Tmg1EncoderTests, Tmg1StreamWriterTests_cs_Tmg1StreamWriterTests [EXTRACTED 0.95]
- **C++ Arduino Decoder Core** — FrequencyModel_h_FrequencyModel, RangeDecoder_h_RangeDecoder, RiceBitReader_h_RiceBitReader, RiceBitReader_cpp_RiceBitReader, Tmg1Decoder_h_Tmg1Decoder, Tmg1Decoder_cpp_Tmg1Decoder [EXTRACTED 0.95]
- **Entropy Coder Implementations (Range + Rice)** — RangeDecoder_h_RangeDecoder, RiceBitReader_h_RiceBitReader, RiceBitReader_cpp_RiceBitReader, FrequencyModel_h_FrequencyModel [INFERRED 0.85]
- **All Tmg1Decoder Tests Registered in Runner** — test_main_runner, test_tmg1decoder_instance_creation, test_tmg1decoder_readfileheader_success, test_tmg1decoder_readfileheader_invalid_signature, test_tmg1decoder_readfileheader_invalid_version, test_tmg1decoder_readfileheader_read_error, test_tmg1decoder_decode_simple_frames, test_tmg1decoder_decode_file_from_simple [EXTRACTED 1.00]
- **TMG1 Encoder Compression Techniques** — encoder_concept_seizu_tmg1_core, encoder_concept_rle, encoder_concept_pframe, encoder_concept_vfr, encoder_concept_scd, encoder_concept_golomb_rice, encoder_concept_range_coder, encoder_concept_iframe [EXTRACTED 1.00]
- **Decoder Project Design Goals** — claude_md, decoder_concept_esp32, decoder_concept_littlefs, decoder_concept_unity_framework, decoder_concept_platformio, decoder_concept_tmg1_format, encoder_concept_tmg1encoder_dotnet [EXTRACTED 1.00]
- **Encoder .NET Solution Projects** — project_seizu_tmg1_cli, project_seizu_tmg1_core, project_seizu_tmg1_tests [INFERRED 0.95]
- **Test Infrastructure Packages** — pkg_nunit, pkg_nunit3testadapter, pkg_ms_net_test_sdk, pkg_coverlet_collector, pkg_coverlet_msbuild, pkg_junitxml_testlogger [INFERRED 0.95]

## Communities (14 total, 6 thin omitted)

### Community 0 - "Microsoft Extensions Packages"
Cohesion: 0.07
Nodes (29): Microsoft.Extensions.Configuration 8.0.0, Microsoft.Extensions.Configuration.Abstractions 8.0.0, Microsoft.Extensions.Configuration.Binder 8.0.0, Microsoft.Extensions.Configuration.CommandLine 8.0.0, Microsoft.Extensions.Configuration.EnvironmentVariables 8.0.0, Microsoft.Extensions.Configuration.FileExtensions 8.0.0, Microsoft.Extensions.Configuration.Json 8.0.0, Microsoft.Extensions.Configuration.UserSecrets 8.0.0 (+21 more)

### Community 1 - "TMG1 Core Codec (C#)"
Cohesion: 0.15
Nodes (23): EncodeResult, EntropyCoderType, FrameType, FrequencyModel, IBitplaneFrameReader, IBitplaneFrameWriter, IEntropyReader, IEntropyWriter (+15 more)

### Community 2 - "Build & Test Infrastructure"
Cohesion: 0.09
Nodes (23): ConsoleAppFramework 5.6.2, coverlet.collector 6.0.2, coverlet.msbuild 6.0.2, JunitXml.TestLogger 3.1.12, Microsoft.ApplicationInsights 2.23.0, Microsoft.CodeCoverage 17.12.0, Microsoft.NET.Test.Sdk 17.12.0, Microsoft.Testing.Extensions.Telemetry 1.9.0 (+15 more)

### Community 3 - "Documentation & Format Concepts"
Cohesion: 0.20
Nodes (21): CLAUDE.md Project Instructions, ESP32 Microcontroller Target, LittleFS Filesystem, TMG1 Format Specification, Encoder CHANGELOG, Golomb-Rice Entropy Coding, P-Frame Inter-frame Differential Compression, Range Coder Entropy Coding (+13 more)

### Community 4 - "Arduino Decoder Tests"
Cohesion: 0.14
Nodes (16): Stream (Arduino Stub), PlatformIO Build System, Unity Test Framework, Decoder GitLab CI Pipeline, I-Frame (Intra-frame), Test Main Runner, test_decode_file_from_simple_tmg1, test_decode_simple_frames (+8 more)

### Community 5 - "C++ Decoder Implementation"
Cohesion: 0.21
Nodes (13): FrequencyModelTests (C# Test Fixture), FrequencyModel (C++ Header), MemoryStream (Test Helper), PredictionFilterTests (C# Test Fixture), RangeCoderTests (C# Test Fixture), RangeDecoder (C++ Header), RiceBitReader (C++ Implementation), RiceBitReader (C++ Header) (+5 more)

### Community 6 - "TMG1 Encoder & Stream Writer"
Cohesion: 0.43
Nodes (7): PayloadCompressorTests (C# Test Fixture), Tmg1EncoderOptions (C# Record), Tmg1EncoderTests (C# Test Fixture), Tmg1Encoder (C# Encoder), Tmg1StreamWriterTests (C# Test Fixture), Tmg1StreamWriter (C# Internal Class), Seizu.Tmg1.Tests Project

### Community 7 - ".NET Solution Structure"
Cohesion: 1.00
Nodes (3): Seizu.Tmg1.Cli Project, Seizu.Tmg1.Core Project, Seizu.Tmg1 Solution

## Ambiguous Edges - Review These
- `TMG1 Decoder README` → `TMG1 Format Specification`  [AMBIGUOUS]
  README.md · relation: references

## Knowledge Gaps
- **62 isolated node(s):** `DevContainer Configuration`, `EncodeResult`, `IBitplaneFrameReader`, `FrequencyModelTests (C# Test Fixture)`, `NUnitSettings (Test Assembly Config)` (+57 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **6 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `TMG1 Decoder README` and `TMG1 Format Specification`?**
  _Edge tagged AMBIGUOUS (relation: references) - confidence is low._
- **Why does `Microsoft.Extensions.Hosting 8.0.0` connect `Microsoft Extensions Packages` to `Build & Test Infrastructure`?**
  _High betweenness centrality (0.103) - this node is a cross-community bridge._
- **Why does `Seizu.Tmg1.Cli` connect `Build & Test Infrastructure` to `Microsoft Extensions Packages`?**
  _High betweenness centrality (0.067) - this node is a cross-community bridge._
- **Are the 5 inferred relationships involving `Tmg1Decoder (C++ Implementation)` (e.g. with `MemoryStream (Test Helper)` and `PredictionFilterTests (C# Test Fixture)`) actually correct?**
  _`Tmg1Decoder (C++ Implementation)` has 5 INFERRED edges - model-reasoned connections that need verification._
- **What connects `DevContainer Configuration`, `EncodeResult`, `IBitplaneFrameReader` to the rest of the system?**
  _65 weakly-connected nodes found - possible documentation gaps or missing edges._
- **Should `Microsoft Extensions Packages` be split into smaller, more focused modules?**
  _Cohesion score 0.07142857142857142 - nodes in this community are weakly interconnected._
- **Should `Build & Test Infrastructure` be split into smaller, more focused modules?**
  _Cohesion score 0.09486166007905138 - nodes in this community are weakly interconnected._