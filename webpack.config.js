const webpack = require('webpack')
const TerserPlugin = require('terser-webpack-plugin')
const path = require('path')

const babelConfig = require('./babel.config')

module.exports = {
  mode: 'production',
  devtool: 'inline-source-map',
  entry: __dirname + '/src/Main.bs',
  target: 'web',
  output: {
    path: __dirname + '/build',
    filename: 'Main.js',
    library: 'Mario',
    libraryExport: 'default',
    libraryTarget: 'umd',
  },
  module: {
    rules: [
      {
        test: /\.(ts|tsx|js|jsx)$/,
        include: [__dirname + '/src'],
        use: [
          {
            loader: require.resolve('babel-loader'),
            options: {
              ...babelConfig,
              cacheDirectory: true,
            },
          },
        ],
      },
    ],
  },
  resolve: {
    extensions: ['.ts', '.tsx', '.jsx', '.js', '.json'],
  },
  plugins: [
    new webpack.HashedModuleIdsPlugin(),
    new webpack.optimize.AggressiveMergingPlugin(),
    new webpack.optimize.OccurrenceOrderPlugin(),
  ],
  optimization: {
    minimizer: [new TerserPlugin()],
  },
}
