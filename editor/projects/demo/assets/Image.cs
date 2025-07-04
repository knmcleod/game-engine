Scene: Image.cs
Entities:
  - Entity: 15566579122833536719
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Layout
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 5871882828996325355
      Children:
        - 17669130327706963993
        - 6282653836554225594
        - 15451615327097329974
        - 14453775193983358713
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: GUI
          ID: 2
    GUILayoutComponent:
      Mode: 1
      StartingOffset: [-3.5, 1.5]
      ChildSize: [1, 1]
      ChildPadding: [0, 0]
    GUIImageComponent:
      Color: [1, 1, 1, 1]
      TextureHandle: 0
      TilingFactor: 1
  - Entity: 15451615327097329974
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Checkbox
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 15566579122833536719
    TransformComponent:
      Translation: [-3.5, -0.5, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: GUI
          ID: 2
    GUICheckboxComponent:
      BackgroundColor: [1, 1, 1, 1]
      BackgroundTextureHandle: 0
      DisabledColor: [0.5, 0.5, 0.5, 0.5]
      DisabledTextureHandle: 0
      EnabledColor: [1, 1, 1, 1]
      EnabledTextureHandle: 0
      HoveredColor: [1, 0.309803933, 0.388235301, 1]
      HoveredTextureHandle: 0
      SelectedColor: [1, 0, 0.380392164, 0.501960814]
      SelectedTextureHandle: 0
      ForegroundColor: [0, 0, 0, 0]
      ForegroundTextureHandle: 0
  - Entity: 3740269551476673245
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Input Field
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 5871882828996325355
    TransformComponent:
      Translation: [3, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: GUI
          ID: 2
    GUIInputFieldComponent:
      BackgroundColor: [1, 1, 1, 1]
      BackgroundTextureHandle: 0
      FillBackground: true
      TextSize: [1, 2.30856633]
      TextColor: [0, 0, 0, 1]
      BGColor: [0, 0, 0, 0]
      FontAssetHandle: 18334408789933592280
      KerningOffset: 0
      LineHeightOffset: 0
      Text: "\nTest\nTest\nTest\nTest"
      TextScalar: 0.5
      TextOffset: [0, 0]
      Padding: [0.25, 0.25]
    ScriptComponent:
      AssetHandle: 17295148655943605647
  - Entity: 17669130327706963993
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Green Button
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 15566579122833536719
    TransformComponent:
      Translation: [-3.5, 1.5, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: GUI
          ID: 2
        - Name: Editor
          ID: 0
    GUIButtonComponent:
      TextColor: [0, 0, 0, 1]
      BGColor: [0, 0, 0, 0]
      KerningOffset: 0
      LineHeightOffset: 0
      Text: ""
      FontAssetHandle: 0
      BackgroundTextureHandle: 0
      BackgroundColor: [1, 1, 1, 1]
      DisabledColor: [0.5, 0.5, 0.5, 0.5]
      DisabledTextureHandle: 0
      EnabledColor: [1, 1, 1, 1]
      EnabledTextureHandle: 0
      HoveredColor: [0.176470578, 1, 0.273356259, 1]
      HoveredTextureHandle: 0
      SelectedColor: [0.022202976, 0.377451003, 0, 0.501960814]
      SelectedTextureHandle: 0
      ForegroundTextureHandle: 0
      ForegroundColor: [0, 0, 0, 0]
    ScriptComponent:
      AssetHandle: 2159947313897299825
  - Entity: 6282653836554225594
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Red Button
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 15566579122833536719
    TransformComponent:
      Translation: [-3.5, 0.5, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: GUI
          ID: 2
    GUIButtonComponent:
      TextColor: [0, 0, 0, 1]
      BGColor: [0, 0, 0, 0]
      KerningOffset: 0
      LineHeightOffset: 0
      Text: ""
      FontAssetHandle: 18334408789933592280
      BackgroundTextureHandle: 0
      BackgroundColor: [1, 1, 1, 1]
      DisabledColor: [0.5, 0.5, 0.5, 0.5]
      DisabledTextureHandle: 0
      EnabledColor: [1, 1, 1, 1]
      EnabledTextureHandle: 0
      HoveredColor: [1, 0.295437396, 0.274509788, 1]
      HoveredTextureHandle: 0
      SelectedColor: [0.990196049, 0, 0, 0.501960814]
      SelectedTextureHandle: 0
      ForegroundTextureHandle: 0
      ForegroundColor: [0, 0, 0, 0]
    ScriptComponent:
      AssetHandle: 2159947313897299825
  - Entity: 5871882828996325355
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Canvas
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 5871882828996325355
      Children:
        - 3740269551476673245
        - 15566579122833536719
    TransformComponent:
      Translation: [0, -5, 1]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: GUI
          ID: 2
        - Name: Editor
          ID: 0
    GUICanvasComponent:
      ControlMouse: true
      ShowMouse: true
      Mode: 1
  - Entity: 14453775193983358713
    TagComponent:
      Tag: UI
      ID: 3
    NameComponent:
      Name: Slider
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 15566579122833536719
    TransformComponent:
      Translation: [-3.5, -1.5, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: GUI
          ID: 2
    GUISliderComponent:
      Direction: Left
      Fill: 0.5
      BackgroundColor: [0.0343137383, 0.0343137383, 0.0343137383, 1]
      BackgroundTextureHandle: 0
      DisabledColor: [0.5, 0.5, 0.5, 0.5]
      DisabledTextureHandle: 0
      EnabledColor: [1, 1, 1, 1]
      EnabledTextureHandle: 0
      HoveredColor: [0.580392182, 1, 0.992156863, 1]
      HoveredTextureHandle: 0
      SelectedColor: [0.501960814, 0.501960814, 1, 0.501960814]
      SelectedTextureHandle: 0
      ForegroundColor: [0.13333334, 0.13333334, 0.13333334, 0]
      ForegroundTextureHandle: 0
    ScriptComponent:
      AssetHandle: 7553269979079703358
  - Entity: 12357236579428767702
    TagComponent:
      Tag: Default
      ID: 0
    NameComponent:
      Name: Ball
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 12357236579428767702
    TransformComponent:
      Translation: [3, 1, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: Game
          ID: 1
    CircleRendererComponent:
      Color: [1, 1, 1, 1]
      Radius: 0.5
      Thickness: 1
      Fade: 0
    Rigidbody2DComponent:
      Type: Dynamic
      FixedRotation: false
    CircleCollider2DComponent:
      Offset: [0, 0]
      Radius: 0.5
      Density: 1
      Friction: 0.5
      Restitution: 1
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 12165168401523622704
    TagComponent:
      Tag: Default
      ID: 0
    NameComponent:
      Name: Left Floor
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 12165168401523622704
    TransformComponent:
      Translation: [-5, 2, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 5, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: Game
          ID: 1
    SpriteRendererComponent:
      Color: [0.0352941193, 0.290196091, 0, 1]
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: Static
      FixedRotation: true
    BoxCollider2DComponent:
      Offset: [0, 0]
      ChildSize: [1, 1]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 1129467347664604248
    TagComponent:
      Tag: Default
      ID: 0
    NameComponent:
      Name: Right Floor
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 1129467347664604248
    TransformComponent:
      Translation: [5, 2, 0]
      Rotation: [0, 0, 0]
      Scale: [1, 5, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: Game
          ID: 1
    SpriteRendererComponent:
      Color: [0.0352941193, 0.290196091, 0, 1]
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: Static
      FixedRotation: true
    BoxCollider2DComponent:
      Offset: [0, 0]
      ChildSize: [1, 1]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 5337912456677767708
    TagComponent:
      Tag: Default
      ID: 0
    NameComponent:
      Name: End Floor
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 5337912456677767708
    TransformComponent:
      Translation: [0, 5, 0]
      Rotation: [0, 0, 0]
      Scale: [5, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
    SpriteRendererComponent:
      Color: [0.0352941193, 0.290196091, 0, 1]
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: Static
      FixedRotation: true
    BoxCollider2DComponent:
      Offset: [0, 0]
      ChildSize: [1, 1]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 11325243841916185303
    TagComponent:
      Tag: Default
      ID: 0
    NameComponent:
      Name: Start Floor
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 11325243841916185303
    TransformComponent:
      Translation: [0, 0, 0]
      Rotation: [0, 0, 0]
      Scale: [10, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Editor
          ID: 0
        - Name: Game
          ID: 1
    SpriteRendererComponent:
      Color: [0.0340253711, 0.289215684, 0, 1]
      TilingFactor: 1
    Rigidbody2DComponent:
      Type: Static
      FixedRotation: true
    BoxCollider2DComponent:
      Offset: [0, 0]
      ChildSize: [1, 1]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 7684260973497798982
    TagComponent:
      Tag: Player
      ID: 1
    NameComponent:
      Name: Player
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 7684260973497798982
    TransformComponent:
      Translation: [0, 2, 0]
      Rotation: [0, 0, 15]
      Scale: [1, 1, 1]
      PivotEnum: 0
    AudioSourceComponent:
      AssetHandle: 12083958930263244828
      Gain: 1
      Pitch: 1
      Loop: false
    AudioListenerComponent:
      {}
    RenderComponent:
      Layers:
        - Name: Game
          ID: 1
        - Name: Editor
          ID: 0
    SpriteRendererComponent:
      Color: [1, 0, 1, 1]
      TilingFactor: 1
    TextRendererComponent:
      TextColor: [0, 0, 0, 1]
      BGColor: [0, 0, 0, 0]
      KerningOffset: 0
      LineHeightOffset: 0
      Text: Test
      TextScalar: 0.5
      TextOffset: [0, 0]
      AssetHandle: 18334408789933592280
    ScriptComponent:
      AssetHandle: 12345127586763726501
      ScriptFields:
        - Name: CenterVelocity
          Type: Boolean
          Data: true
        - Name: Color
          Type: Vector4
          Data: [0, 0, 0, 0]
        - Name: Speed
          Type: Float
          Data: 20
    Rigidbody2DComponent:
      Type: Dynamic
      FixedRotation: false
    BoxCollider2DComponent:
      Offset: [0, 0]
      ChildSize: [1, 1]
      Density: 1
      Friction: 0.5
      Restitution: 0
      RestitutionThreshold: 0.5
      Show: false
  - Entity: 11677086725533177951
    TagComponent:
      Tag: Camera
      ID: 2
    NameComponent:
      Name: Camera
    ActiveComponent:
      Active: true
      Hidden: false
    RelationshipComponent:
      Parent: 11677086725533177951
    TransformComponent:
      Translation: [0, -5, 1]
      Rotation: [0, 0, 0]
      Scale: [1, 1, 1]
      PivotEnum: 0
    RenderComponent:
      Layers:
        - Name: Game
          ID: 1
        - Name: GUI
          ID: 2
        - Name: Editor
          ID: 0
    CameraComponent:
      FixedAspectRatio: false
      Primary: true
      Camera:
        Far: 1
        Near: -1
        FOV: 5
        Type: 1
    ScriptComponent:
      AssetHandle: 13426698827862289058
      ScriptFields:
        - Name: FollowDistance
          Type: Float
          Data: 1
        - Name: FollowEntity
          Type: Entity
          Data: 10063102736654795519
        - Name: HeightOffset
          Type: Float
          Data: 0
        - Name: WidthOffset
          Type: Float
          Data: 0