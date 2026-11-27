# The catalog of the AI models (0.99.0)

`models.json` lists the models the settings pages of Read Aloud, Handwriting and Speech Transcription offer; the program
downloads their files from the release named in it (`models-5`) of the project's repository and checks each against its
size and SHA-256. Its format is described in `src/core/AiCatalog.h`. The models are not part of the program and not part
of this repository: each entry names its source, license and credit. The file is written by the workflow that publishes
the models (until then it lists none).
