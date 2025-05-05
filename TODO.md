Need to replace all YAML parsing with XML.

Files where YAML is parsed:

- [x] AssetDescriptor  
- [x] AssetGenerator  
- [x] MaterialParser  
- [x] ProjectDescriptor  
- [x] SceneParser  
- [ ] XEditor

### XPak

- Asset generation is slightly broken. ID values parse as giberish in the XML descriptor and `Source` path strings are full paths instead of relative to the project root.