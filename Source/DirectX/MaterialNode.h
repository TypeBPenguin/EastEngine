#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class Texture;

		namespace EmMaterialNode
		{
			enum OutputType
			{
				eBaseColor = 0,
				eEmissiveColor,
				eNormal,

				eRoughness,
				eMetallic,
				eSubsurface,
				eSpecular,
				eSpecularTint,
				eAnisotropic,
				eSheen,
				eSheenTint,
				eClearcoat,
				eClearcoatGloss,

				OutputTypeCount,
			};
			
			enum InputType
			{
				ePosition = 0,
				eWorldPosition,
				eWorldVIewPosition,
				eWorldViewProjPosition,
				eUV,
				eNormalDirection,
				eTangentDirection,
				eBinormalDirection,
			};

			enum ValueType
			{
			};

			enum NodeType
			{
				eOutput = 0,

				// function
				eAdd,
				eSubtract,
				eMultiply,
				eDivide,
				eReciprocal,
				ePower,
				eSqrt,
				eLog,
				eMin,
				eMax,
				eAbs,
				eSign,
				eCeil,
				eRound,
				eFloor,
				eTrunc,
				eStep,
				eSmoothstep,
				eIf,
				eFrac,
				eFmod,
				eClamp,
				eLerp,
				eAppend,
				ePanner,
				eDot,
				eCross,
				eReflect,
				eNormalize,
				eSin,
				eCos,
				eTan,
				eArcSin,
				eArcCos,
				eArcTan,
				eArcTan2,

				// Value
				eScalar,
				eVector2,
				eVector3,
				eVector4,
				eMatrix,
				eTexture2D,
			};
		}

		class MaterialNode
		{
		private:
			class Interface
			{
			public:
				Interface() {}
				virtual ~Interface() = 0;

				virtual EmMaterialNode::NodeType GetType() = 0;
			};

		public:
			class Output : public Interface
			{
			public:
				Output();
				virtual ~Output();

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eOutput; };

			private:
				std::array<Interface*, EmMaterialNode::OutputTypeCount> m_outputProperty;
			};

			class Value : public Interface
			{
			public:
				Value() {}
				virtual ~Value() = 0;
			};

			class Scalar : public Value
			{
			public:
				Scalar()
					: m_fValue(0.f)
				{
				}
				virtual ~Scalar() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eScalar; };

				float GetValue() const { return m_fValue; }
				void SetValue(float fValue) { m_fValue = fValue; }

			private:
				float m_fValue;
			};

			class Vector2 : public Value
			{
			public:
				Vector2() {}
				virtual ~Vector2() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eVector2; };

				const Math::Vector2& GetValue() const { return m_f2Value; }
				void SetValue(const Math::Vector2& f2Value) { m_f2Value = f2Value; }

			private:
				Math::Vector2 m_f2Value;
			};

			class Vector3 : public Value
			{
			public:
				Vector3() {}
				virtual ~Vector3() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eVector3; };

				const Math::Vector3& GetValue() const { return m_f3Value; }
				void SetValue(const Math::Vector3& f3Value) { m_f3Value = f3Value; }

			private:
				Math::Vector3 m_f3Value;
			};

			class Vector4 : public Value
			{
			public:
				Vector4() {}
				virtual ~Vector4() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eVector4; };

				const Math::Vector4& GetValue() const { return m_f4Value; }
				void SetValue(const Math::Vector4& f4Value) { m_f4Value = f4Value; }

			private:
				Math::Vector4 m_f4Value;
			};

			class Matrix : public Value
			{
			public:
				Matrix() {}
				virtual ~Matrix() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eVector4; };

				const Math::Matrix& GetValue() const { return m_matValue; }
				void SetValue(const Math::Matrix& matValue) { m_matValue = matValue; }

			private:
				Math::Matrix m_matValue;
			};

			class Texture : public Value
			{
			public:
				Texture() {}
				virtual ~Texture() {}

				virtual EmMaterialNode::NodeType GetType() { return EmMaterialNode::NodeType::eTexture2D; };

				std::shared_ptr<Graphics::Texture> GetValue() const { return m_pTexture; }
				void SetValue(std::shared_ptr<Graphics::Texture> pTexture) { m_pTexture = pTexture; }

			private:
				std::shared_ptr<Graphics::Texture> m_pTexture;
			};

			class Function : public Interface
			{
			public:
				Function() {}
				virtual ~Function() = 0;
			};

			class FuncAdd : public Function
			{
			public:
				FuncAdd()
					: m_pValueA(nullptr)
					, m_pValueB(nullptr)
				{
				}

				virtual ~FuncAdd() {}

				void SetValueA(Value* pValueA) { m_pValueA = pValueA; }
				void SetValueB(Value* pValueB) { m_pValueB = pValueB; }

			private:
				Value* m_pValueA;
				Value* m_pValueB;
			};
			
			class FuncSubtract : public Function
			{
			public:
				FuncSubtract() {}
				virtual ~FuncSubtract() {}
			};
			
			class FuncMultiply : public Function
			{
			public:
				FuncMultiply() {}
				virtual ~FuncMultiply() {}
			};
			
			class FuncDivide : public Function
			{
			public:
				FuncDivide() {}
				virtual ~FuncDivide() {}
			};
			
			class FuncReciprocal : public Function
			{
			public:
				FuncReciprocal() {}
				virtual ~FuncReciprocal() {}
			};

			class FuncPower : public Function
			{
			public:
				FuncPower() {}
				virtual ~FuncPower() {}
			};

			class FuncSqrt : public Function
			{
			public:
				FuncSqrt() {}
				virtual ~FuncSqrt() {}
			};

			class FuncLog : public Function
			{
			public:
				FuncLog() {}
				virtual ~FuncLog() {}
			};

			class FuncMin : public Function
			{
			public:
				FuncMin() {}
				virtual ~FuncMin() {}
			};

			class FuncMax : public Function
			{
			public:
				FuncMax() {}
				virtual ~FuncMax() {}
			};

			class FuncAbs : public Function
			{
			public:
				FuncAbs() {}
				virtual ~FuncAbs() {}
			};

			class FuncSign : public Function
			{
			public:
				FuncSign() {}
				virtual ~FuncSign() {}
			};

			class FuncCeil : public Function
			{
			public:
				FuncCeil() {}
				virtual ~FuncCeil() {}
			};

			class FuncRound : public Function
			{
			public:
				FuncRound() {}
				virtual ~FuncRound() {}
			};

			class FuncTrunc : public Function
			{
			public:
				FuncTrunc() {}
				virtual ~FuncTrunc() {}
			};

			class FuncStep : public Function
			{
			public:
				FuncStep() {}
				virtual ~FuncStep() {}
			};

			class FuncSmoothstep : public Function
			{
			public:
				FuncSmoothstep() {}
				virtual ~FuncSmoothstep() {}
			};

			class FuncIf : public Function
			{
			public:
				FuncIf() {}
				virtual ~FuncIf() {}
			};

			class FuncFmod : public Function
			{
			public:
				FuncFmod() {}
				virtual ~FuncFmod() {}
			};

			class FuncClamp : public Function
			{
			public:
				FuncClamp() {}
				virtual ~FuncClamp() {}
			};

			class FuncLerp : public Function
			{
			public:
				FuncLerp() {}
				virtual ~FuncLerp() {}
			};

			class FuncAppend : public Function
			{
			public:
				FuncAppend() {}
				virtual ~FuncAppend() {}
			};

			class FuncPanner : public Function
			{
			public:
				FuncPanner() {}
				virtual ~FuncPanner() {}
			};

			class FuncDot : public Function
			{
			public:
				FuncDot() {}
				virtual ~FuncDot() {}
			};

			class FuncCross : public Function
			{
			public:
				FuncCross() {}
				virtual ~FuncCross() {}
			};

			class FuncReflect : public Function
			{
			public:
				FuncReflect() {}
				virtual ~FuncReflect() {}
			};

			class FuncNormalize : public Function
			{
			public:
				FuncNormalize() {}
				virtual ~FuncNormalize() {}
			};

			class FuncSin : public Function
			{
			public:
				FuncSin() {}
				virtual ~FuncSin() {}
			};

			class FuncCos : public Function
			{
			public:
				FuncCos() {}
				virtual ~FuncCos() {}
			};

			class FuncTan : public Function
			{
			public:
				FuncTan() {}
				virtual ~FuncTan() {}
			};

			class FuncArcSin : public Function
			{
			public:
				FuncArcSin() {}
				virtual ~FuncArcSin() {}
			};

			class FuncArcCos : public Function
			{
			public:
				FuncArcCos() {}
				virtual ~FuncArcCos() {}
			};

			class FuncArcTan : public Function
			{
			public:
				FuncArcTan() {}
				virtual ~FuncArcTan() {}
			};

			class FuncArcTan2 : public Function
			{
			public:
				FuncArcTan2() {}
				virtual ~FuncArcTan2() {}
			};

		public:
			MaterialNode();
			virtual ~MaterialNode();
		};
	}
}